/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <uv.h>
#include <artik_loop.h>

#include "os_loop.h"

struct _timeout {
	timeout_callback func;
	void *user_data;
	uv_timer_t *handle;
	int id;
};

struct _periodic {
	periodic_callback func;
	void *user_data;
	uv_timer_t *handle;
	int id;
};

struct _watch {
	watch_callback func;
	void *user_data;
	uv_poll_t *handle;
	int fd;
	int id;
};

struct _idle {
	idle_callback func;
	void *user_data;
	uv_idle_t *handle;
	int id;
};

struct _vector {
	void *data;
	uint16_t len;
	uint16_t elem_size;
};

struct _ptr_vector {
	struct _vector base;
};

static uv_loop_t *_loop = NULL;

static int _id_timeout;
static int _id_periodic;
static int _id_watch;
static int _id_idle;

static struct _ptr_vector timeout_vector = { { NULL, 0, sizeof(void *) } };
static struct _ptr_vector periodic_vector = { { NULL, 0, sizeof(void *) } };
static struct _ptr_vector watch_vector = { { NULL, 0, sizeof(void *) } };
static struct _ptr_vector idle_vector = { { NULL, 0, sizeof(void *) } };

int _util_size_mul(size_t elem_size, size_t num_elems, size_t *out)
{
	*out = elem_size * num_elems;

	return 0;
}

#define _LIKELY(x)		__builtin_expect(!!(x), 1)
#define _UNLIKELY(x)	__builtin_expect(!!(x), 0)

/* Power of 2 alignment */
#define DEFINE_ALIGN_POWER2(name_, type_, max_, clz_fn_) \
	static inline type_ \
	name_(type_ u) \
	{ \
		unsigned int left_zeros; \
		if (u <= 1) /* 0 is undefined for __builtin_clz() */ \
			return u; \
		left_zeros = clz_fn_(u - 1); \
		if (_UNLIKELY(left_zeros <= 1)) \
			return max_; \
		return (type_)1 << ((sizeof(u) * 8) - left_zeros); \
	}

DEFINE_ALIGN_POWER2(align_power2_uint, unsigned int, UINT_MAX, __builtin_clz)
DEFINE_ALIGN_POWER2(align_power2_size, size_t, SIZE_MAX, __builtin_clzl)

#undef DEFINE_ALIGN_POWER2

static inline int
align_power2_short_uint(unsigned short u)
{
	unsigned int aligned = align_power2_uint(u);

	return _LIKELY(aligned <= USHRT_MAX) ? aligned : USHRT_MAX;
}

#define align_power2(u_) \
({ \
	__typeof__((u_) + 0)pow2__aligned; /* + 0 to remove const */ \
	if (__builtin_types_compatible_p(__typeof__(u_), size_t)) { \
		pow2__aligned = align_power2_size(u_); \
	} else if (__builtin_types_compatible_p(__typeof__(u_), unsigned int)) { \
		pow2__aligned = align_power2_uint(u_); \
	} else if (__builtin_types_compatible_p(__typeof__(u_), unsigned short)) { \
		pow2__aligned = align_power2_short_uint(u_); \
	} else { \
		__builtin_unreachable(); \
	} \
	pow2__aligned; \
})

int _vector_grow(struct _vector *v, uint16_t amount)
{
	size_t new_cap, old_cap;
	uint16_t new_len;

	new_len = v->len + amount;
	old_cap = align_power2(v->len);
	new_cap = align_power2(new_len);

	if (new_cap != old_cap) {
		void *data;
		size_t data_size;

		int r = _util_size_mul(v->elem_size, new_cap, &data_size);

		if (r < 0)
			return E_INVALID_VALUE;

		data = realloc(v->data, data_size);
		v->data = data;
	}

	v->len = new_len;
	return 0;
}

void *_vector_append_n(struct _vector *v, uint16_t n)
{
	int err = _vector_grow(v, n);

	if (err < 0)
		return NULL;

	void *new_elems = (unsigned char *)v->data + (v->elem_size * (v->len - n));

	memset(new_elems, 0, (size_t)v->elem_size * n);

	return new_elems;
}

void *_vector_append(struct _vector *v)
{
	return _vector_append_n(v, 1);
}

int _ptr_vector_append(struct _ptr_vector *pv, const void *ptr)
{
	void **data;

	data = _vector_append(&pv->base);
	*data = (void *)ptr;
	return 0;
}

void *_vector_get_no_check(const struct _vector *v, uint16_t i)
{
	const unsigned char *data;

	data = (const unsigned char *)v->data;

	return (void *)&data[v->elem_size * i];
}

void *_ptr_vector_get_no_check(const struct _ptr_vector *pv, uint16_t i)
{
	void **data;

	data = (void **)_vector_get_no_check(&pv->base, i);
	return *data;
}

#define _PTR_VECTOR_FOREACH_REVERSE_IDX(vector, itrvar, idx) \
	for (idx = (vector)->base.len - 1; \
		idx != ((__typeof__(idx)) -1) && \
		(itrvar = (__typeof__(itrvar))_ptr_vector_get_no_check((vector), idx), true); \
		idx--)

void _vector_shrink(struct _vector *v)
{
	unsigned int new_cap, old_cap;
	void *data;

	if (v->len == 0) {
		free(v->data);
		v->data = NULL;
		return;
	}

	old_cap = align_power2(v->len + 1U);
	new_cap = align_power2(v->len);
	if (new_cap == old_cap)
		return;

	data = realloc(v->data, new_cap * v->elem_size);
	if (!data)
		return;

	v->data = data;
}

int _vector_del(struct _vector *v, uint16_t i)
{
	size_t tail_len;

	if (i >= v->len)
		return E_INVALID_VALUE;

	v->len--;
	tail_len = v->len - i;

	if (tail_len) {
		unsigned char *data, *dst, *src;

		data = v->data;
		dst = &data[v->elem_size * i];
		src = dst + v->elem_size;
		memmove(dst, src, v->elem_size * tail_len);
	}

	_vector_shrink(v);
	return 0;
}

int _ptr_vector_del(struct _ptr_vector *pv, uint16_t i)
{
	return _vector_del(&pv->base, i);
}

static void _check_loop(void)
{
	if (_loop == NULL)
		_loop = uv_default_loop();
}

static uv_loop_t *_get_loop(void)
{
	_check_loop();
	return _loop;
}

void timeout_cleanup(int timeout_id)
{
	struct _timeout *timeout;
	uint16_t i;

	_PTR_VECTOR_FOREACH_REVERSE_IDX(&timeout_vector, timeout, i) {
		if (timeout->id == timeout_id) {
			if (timeout->handle) {
				uv_timer_stop(timeout->handle);
				free(timeout->handle);

				memset(timeout, 0, sizeof(struct _timeout));
				free(timeout);
			}

			_ptr_vector_del(&timeout_vector, i);
		}
	}
}

static void _timeout_callback(uv_timer_t *handle)
{
	struct _timeout *timeout = handle->data;

	if (timeout->func)
		timeout->func(timeout->user_data);

	uv_timer_stop(timeout->handle);
	free(timeout->handle);

	memset(timeout, 0, sizeof(struct _timeout));
	free(timeout);
	timeout_cleanup(timeout->id);
}

artik_error os_add_timeout_callback(int *timeout_id, unsigned int msec,
		timeout_callback func, void *user_data)
{
	int ret;
	struct _timeout *timeout = NULL;

	timeout = malloc(sizeof(struct _timeout));
	timeout->func = func;
	timeout->user_data = user_data;
	timeout->handle = malloc(sizeof(uv_timer_t));
	timeout->handle->data = timeout;

	uv_update_time(_get_loop());
	uv_timer_init(_get_loop(), timeout->handle);
	uv_timer_start(timeout->handle, _timeout_callback, msec, 0);

	ret = _ptr_vector_append(&timeout_vector, timeout);
	if (ret != 0)
		return E_INVALID_VALUE;

	if (_id_timeout >= 255)
		_id_timeout = 0;
	timeout->id = _id_timeout++;
	*timeout_id = timeout->id;

	return S_OK;
}

artik_error os_remove_timeout_callback(int timeout_id)
{
	timeout_cleanup(timeout_id);

	return S_OK;
}

void periodic_cleanup(int periodic_id)
{
	struct _periodic *periodic;
	uint16_t i;

	_PTR_VECTOR_FOREACH_REVERSE_IDX(&periodic_vector, periodic, i) {
		if (periodic->id == periodic_id) {
			if (periodic->handle) {
				uv_timer_stop(periodic->handle);
				free(periodic->handle);

				memset(periodic, 0, sizeof(struct _periodic));
				free(periodic);
			}

			_ptr_vector_del(&periodic_vector, i);
		}
	}
}

static void _periodic_callback(uv_timer_t *handle)
{
	struct _periodic *periodic = handle->data;
	int ret;

	if (periodic->func) {
		ret = periodic->func(periodic->user_data);
		if (ret == 1)
			return;
	}

	uv_timer_stop(periodic->handle);
	free(periodic->handle);

	memset(periodic, 0, sizeof(struct _periodic));
	free(periodic);
	periodic_cleanup(periodic->id);
}

artik_error os_add_periodic_callback(int *periodic_id, unsigned int msec,
		periodic_callback func, void *user_data)
{
	int ret;
	struct _periodic *periodic = NULL;

	periodic = malloc(sizeof(struct _timeout));
	periodic->func = func;
	periodic->user_data = user_data;
	periodic->handle = malloc(sizeof(uv_timer_t));
	periodic->handle->data = periodic;

	uv_update_time(_get_loop());
	uv_timer_init(_get_loop(), periodic->handle);
	uv_timer_start(periodic->handle, _periodic_callback, msec, 1);

	ret = _ptr_vector_append(&periodic_vector, periodic);
	if (ret != 0)
		return E_INVALID_VALUE;

	if (_id_periodic >= 255)
		_id_periodic = 0;
	periodic->id = _id_periodic++;
	*periodic_id = periodic->id;

	return S_OK;
}

artik_error os_remove_periodic_callback(int periodic_id)
{
	periodic_cleanup(periodic_id);

	return S_OK;
}

void os_loop_run(void)
{
	uv_run(_get_loop(), UV_RUN_DEFAULT);
}

void os_loop_quit(void)
{
	if (_loop)
		uv_loop_close(_loop);

	_loop = NULL;
}

void watch_cleanup(int watch_id)
{
	struct _watch *watch;
	uint16_t i;

	_PTR_VECTOR_FOREACH_REVERSE_IDX(&watch_vector, watch, i) {
		if (watch->id == watch_id) {
			if (watch->handle) {
				uv_poll_stop(watch->handle);
				free(watch->handle);

				memset(watch, 0, sizeof(struct _watch));
				free(watch);
			}

			_ptr_vector_del(&watch_vector, i);
		}
	}
}

static void _watch_callback(uv_poll_t *handle, int status, int events)
{
	struct _watch *watch = handle->data;
	enum watch_io io = 0;
	int ret;

	if (events & UV_READABLE)
		io |= WATCH_IO_IN;
	if (events & UV_WRITABLE)
		io |= WATCH_IO_OUT;
	if (events & UV_DISCONNECT)
		io |= WATCH_IO_HUP;

	ret = watch->func(watch->fd, io, watch->user_data);
	if (!ret) {
		uv_poll_stop(watch->handle);
		free(watch->handle);

		memset(watch, 0, sizeof(struct _watch));
		free(watch);
		watch_cleanup(watch->id);
	}
}

artik_error os_add_fd_watch(int fd, enum watch_io io, watch_callback func,
		void *user_data, int *watch_id)
{
	int ret;
	struct _watch *watch;
	int events = 0;

	watch = malloc(sizeof(struct _watch));
	watch->func = func;
	watch->user_data = user_data;
	watch->handle = malloc(sizeof(uv_poll_t));
	watch->handle->data = watch;

	if (io & WATCH_IO_IN)
		events |= UV_READABLE;
	if (io & WATCH_IO_OUT)
		events |= UV_WRITABLE;
	if (io & WATCH_IO_HUP)
		events |= UV_DISCONNECT;

	uv_poll_init_socket(_get_loop(), watch->handle, fd);
	uv_poll_start(watch->handle, events, _watch_callback);

	ret = _ptr_vector_append(&watch_vector, watch);
	if (ret != 0)
		return E_INVALID_VALUE;

	if (_id_watch >= 255)
		_id_watch = 0;
	watch->id = _id_watch++;
	*watch_id = watch->id;

	watch->fd = fd;

	return S_OK;
}

artik_error os_remove_fd_watch(int watch_id)
{
	watch_cleanup(watch_id);

	return S_OK;
}

void idle_cleanup(int idle_id)
{
	struct _idle *idle;
	uint16_t i;

	_PTR_VECTOR_FOREACH_REVERSE_IDX(&idle_vector, idle, i) {
		if (idle->id == idle_id) {
			if (idle->handle) {
				uv_idle_stop(idle->handle);
				free(idle->handle);

				memset(idle, 0, sizeof(struct _idle));
				free(idle);
			}

			_ptr_vector_del(&idle_vector, i);
		}
	}
}

static void _idle_callback(uv_idle_t *handle)
{
	struct _idle *idle = handle->data;
	int ret;

	ret = idle->func(idle->user_data);
	if (!ret) {
		uv_idle_stop(idle->handle);
		free(idle->handle);

		memset(idle, 0, sizeof(struct _idle));
		free(idle);
		idle_cleanup(idle->id);
	}
}

artik_error os_add_idle_callback(int *idle_id, idle_callback func, void *user_data)
{
	int ret;
	struct _idle *idle;

	idle = malloc(sizeof(struct _idle));
	idle->func = func;
	idle->user_data = user_data;
	idle->handle = malloc(sizeof(uv_idle_t));
	idle->handle->data = idle;

	uv_idle_init(_get_loop(), idle->handle);
	uv_idle_start(idle->handle, _idle_callback);

	ret = _ptr_vector_append(&idle_vector, idle);
	if (ret != 0)
		return E_INVALID_VALUE;

	if (_id_idle >= 255)
		_id_idle = 0;

	*idle_id = _id_idle++;

	return S_OK;
}

artik_error os_remove_idle_callback(int idle_id)
{
	idle_cleanup(idle_id);

	return S_OK;
}
