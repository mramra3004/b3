/******************************************************************************
  This file is part of b3.

  Copyright 2020 Richard Paul Baeck <richard.baeck@mailbox.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*******************************************************************************/

/**
 * @author Richard B�ck <richard.baeck@mailbox.org>
 * @date 2020-02-26
 * @brief File contains the key binding director command class implementation and its private methods
 */

#include "kc_director.h"

#include <wbkbase/logger.h>

static wbk_logger_t logger =  { "kc_director" };

static int
b3_kc_director_exec_cw(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_cm(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_mawtw(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_awtf(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_mawu(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_mawd(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_mawl(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_mawr(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_sawu(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_sawd(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_sawl(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_sawr(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_tawf(b3_kc_director_t *kc_director);

static int
b3_kc_director_exec_caw(b3_kc_director_t *kc_director);

b3_kc_director_t *
b3_kc_director_new(wbk_b_t *comb, b3_director_t *director, b3_kc_director_kind_t kind, void *data)
{
	b3_kc_director_t *kc_director;
	int length;

	kc_director = NULL;
	kc_director = malloc(sizeof(b3_kc_director_t));
	memset(kc_director, 0, sizeof(b3_kc_director_t));

	if (kc_director != NULL) {
		kc_director->kc = wbk_kc_new(comb);

		kc_director->global_mutex = CreateMutex(NULL, FALSE, NULL);

		kc_director->director = director;

		kc_director->kind = kind;

		kc_director->data = data;
	}

	return kc_director;
}

int
b3_kc_director_free(b3_kc_director_t *kc_director)
{
	wbk_kc_free(kc_director->kc);
	kc_director->kc = NULL;

	ReleaseMutex(kc_director->global_mutex);
	CloseHandle(kc_director->global_mutex);

	kc_director->director = NULL;

	if (kc_director->data) {
		switch (kc_director->kind) {
		case CHANGE_WORKSPACE:
			free(kc_director->data);
			kc_director->data = NULL;
			break;

		case CHANGE_MONITOR:
			free(kc_director->data);
			kc_director->data = NULL;
			break;

		case MOVE_ACTIVE_WINDOW_TO_WORKSPACE:
			free(kc_director->data);
			kc_director->data = NULL;
			break;

		case ACTIVE_WINDOW_TOGGLE_FLOATING:
			kc_director->data = NULL;
			break;
		}
	}

	kc_director->data = NULL;

	free(kc_director);

	return 0;
}

const wbk_b_t *
b3_kc_director_get_binding(const b3_kc_director_t *kc)
{
	return wbk_kc_get_binding(kc->kc);
}

int
b3_kc_director_exec(b3_kc_director_t *kc_director)
{
	int ret;

	WaitForSingleObject(kc_director->global_mutex, INFINITE);

#ifdef DEBUG_ENABLED
	char *binding;

	binding = wbk_b_to_str(b3_kc_director_get_binding(kc_director));
	wbk_logger_log(&logger, DEBUG, "Exec binding: %s\n", binding);
	free(binding);
	binding = NULL;
#endif

	switch(kc_director->kind) {
	case CHANGE_WORKSPACE:
		ret = b3_kc_director_exec_cw(kc_director);
		break;

	case CHANGE_MONITOR:
		ret = b3_kc_director_exec_cm(kc_director);
		break;

	case MOVE_ACTIVE_WINDOW_TO_WORKSPACE:
		ret = b3_kc_director_exec_mawtw(kc_director);
		break;

	case ACTIVE_WINDOW_TOGGLE_FLOATING:
		ret = b3_kc_director_exec_awtf(kc_director);
		break;

	case MOVE_ACTIVE_WINDOW_UP:
		ret = b3_kc_director_exec_mawu(kc_director);
		break;

	case MOVE_ACTIVE_WINDOW_DOWN:
		ret = b3_kc_director_exec_mawd(kc_director);
		break;

	case MOVE_ACTIVE_WINDOW_LEFT:
		ret = b3_kc_director_exec_mawl(kc_director);
		break;

	case MOVE_ACTIVE_WINDOW_RIGHT:
		ret = b3_kc_director_exec_mawr(kc_director);
		break;

	case SET_ACTIVE_WINDOW_UP:
		ret = b3_kc_director_exec_sawu(kc_director);
		break;

	case SET_ACTIVE_WINDOW_DOWN:
		ret = b3_kc_director_exec_sawd(kc_director);
		break;

	case SET_ACTIVE_WINDOW_LEFT:
		ret = b3_kc_director_exec_sawl(kc_director);
		break;

	case SET_ACTIVE_WINDOW_RIGHT:
		ret = b3_kc_director_exec_sawr(kc_director);
		break;

	case TOGGLE_ACTIVE_WINDOW_FULLSCREEN:
		ret = b3_kc_director_exec_tawf(kc_director);
		break;

	case CLOSE_ACTIVE_WINDOW:
		ret = b3_kc_director_exec_caw(kc_director);
		break;

	default:
		ret = -1;
		// TODO
	}

	ReleaseMutex(kc_director->global_mutex);

	return ret;
}

int
b3_kc_director_exec_cw(b3_kc_director_t *kc_director)
{
	int ret;
	char *ws_id;

	ws_id = kc_director->data;

	ret = b3_director_switch_to_ws(kc_director->director, ws_id);

	return ret;
}

int
b3_kc_director_exec_cm(b3_kc_director_t *kc_director)
{
	int ret;
	char *monitor_name;

	monitor_name = kc_director->data;

	ret = b3_director_set_focused_monitor(kc_director->director, monitor_name);

	return ret;
}

int
b3_kc_director_exec_mawtw(b3_kc_director_t *kc_director)
{
	int ret;
	char *ws_id;

	ws_id = kc_director->data;

	ret = b3_director_move_active_win_to_ws(kc_director->director, ws_id);

	return ret;
}

int
b3_kc_director_exec_awtf(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_active_win_toggle_floating(kc_director->director);

	return ret;
}

int
b3_kc_director_exec_mawu(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_move_active_win(kc_director->director, UP);

	return ret;
}

int
b3_kc_director_exec_mawd(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_move_active_win(kc_director->director, DOWN);

	return ret;
}

int
b3_kc_director_exec_mawl(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_move_active_win(kc_director->director, LEFT);

	return ret;
}

int
b3_kc_director_exec_mawr(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_move_active_win(kc_director->director, RIGHT);

	return ret;
}

int
b3_kc_director_exec_sawu(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_set_active_win_by_direction(kc_director->director, UP);

	return ret;
}

int
b3_kc_director_exec_sawd(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_set_active_win_by_direction(kc_director->director, DOWN);

	return ret;
}

int
b3_kc_director_exec_sawl(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_set_active_win_by_direction(kc_director->director, LEFT);

	return ret;
}

int
b3_kc_director_exec_sawr(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_set_active_win_by_direction(kc_director->director, RIGHT);

	return ret;
}

int
b3_kc_director_exec_tawf(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_toggle_active_win_fullscreen(kc_director->director);

	return ret;
}

int
b3_kc_director_exec_caw(b3_kc_director_t *kc_director)
{
	int ret;

	ret = b3_director_close_active_win(kc_director->director);

	return ret;
}
