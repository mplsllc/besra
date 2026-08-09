/*
 * QuickJS implementation of the engine-agnostic javascript/js.h interface.
 *
 * This is the runtime lifecycle layer only: heap/thread/context creation,
 * script execution, and a minimal hand-written `console` object (enough to
 * prove the pipeline end to end). It does not yet expose the DOM to script
 * (window/document/element bindings) -- that's the much larger follow-on
 * piece, replacing what nsgenbind + the duktape .bnd files used to
 * generate for Duktape. See docs/roadmap.md and plan.md for status.
 */

#include <stdlib.h>
#include <string.h>

#include <quickjs.h>

#include "utils/errors.h"
#include "utils/log.h"
#include "netsurf/browser_window.h"
#include "netsurf/console.h"

#include "javascript/js.h"
#include "javascript/content.h"

struct jsheap {
	JSRuntime *rt;
};

struct jsthread {
	JSContext *ctx;
	struct browser_window *win;	/* win_priv, NULL until DOM bindings exist */
	void *doc_priv;
	bool closed;
};

/**
 * Shared implementation for console.log/info/warn/error/debug: join all
 * arguments as strings (space separated, matching console.* semantics)
 * and forward to the core's console log sink.
 */
static JSValue
console_write(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv,
		browser_window_console_flags level)
{
	struct jsthread *thread = JS_GetContextOpaque(ctx);
	size_t total_len = 0;
	char *buf = NULL;
	int i;

	(void)this_val;

	for (i = 0; i < argc; i++) {
		size_t part_len;
		const char *part = JS_ToCStringLen(ctx, &part_len, argv[i]);
		if (part == NULL) {
			continue;
		}

		size_t sep_len = (i > 0) ? 1 : 0;
		char *grown = realloc(buf, total_len + sep_len + part_len + 1);
		if (grown == NULL) {
			JS_FreeCString(ctx, part);
			free(buf);
			return JS_ThrowOutOfMemory(ctx);
		}
		buf = grown;

		if (sep_len) {
			buf[total_len] = ' ';
		}
		memcpy(buf + total_len + sep_len, part, part_len);
		total_len += sep_len + part_len;
		buf[total_len] = '\0';

		JS_FreeCString(ctx, part);
	}

	if (buf == NULL) {
		buf = strdup("");
		total_len = 0;
	}

	if (thread != NULL && thread->win != NULL && !thread->closed) {
		browser_window_console_log(thread->win, BW_CS_SCRIPT_CONSOLE,
				buf, total_len, level | BW_CS_FLAG_FOLDABLE);
	} else {
		/* No browser window attached to this context yet (DOM
		 * bindings not wired up): still surface the message so
		 * script output is visible during bring-up. */
		NSLOG(netsurf, INFO, "console: %s", buf);
	}

	free(buf);
	return JS_UNDEFINED;
}

static JSValue js_console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
	return console_write(ctx, this_val, argc, argv, BW_CS_FLAG_LEVEL_LOG);
}

static JSValue js_console_info(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
	return console_write(ctx, this_val, argc, argv, BW_CS_FLAG_LEVEL_INFO);
}

static JSValue js_console_warn(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
	return console_write(ctx, this_val, argc, argv, BW_CS_FLAG_LEVEL_WARN);
}

static JSValue js_console_error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
	return console_write(ctx, this_val, argc, argv, BW_CS_FLAG_LEVEL_ERROR);
}

static JSValue js_console_debug(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
	return console_write(ctx, this_val, argc, argv, BW_CS_FLAG_LEVEL_DEBUG);
}

static void install_console(JSContext *ctx)
{
	JSValue global = JS_GetGlobalObject(ctx);
	JSValue console = JS_NewObject(ctx);

	JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, js_console_log, "log", 0));
	JS_SetPropertyStr(ctx, console, "info", JS_NewCFunction(ctx, js_console_info, "info", 0));
	JS_SetPropertyStr(ctx, console, "warn", JS_NewCFunction(ctx, js_console_warn, "warn", 0));
	JS_SetPropertyStr(ctx, console, "error", JS_NewCFunction(ctx, js_console_error, "error", 0));
	JS_SetPropertyStr(ctx, console, "debug", JS_NewCFunction(ctx, js_console_debug, "debug", 0));

	JS_SetPropertyStr(ctx, global, "console", console);
	JS_FreeValue(ctx, global);
}

/* exported interface documented in javascript/js.h */
void js_initialise(void)
{
	/* registers the text/javascript etc. content handler so <script>
	 * tags' inline/external content is routed to js_exec() at all. */
	javascript_init();
}

/* exported interface documented in javascript/js.h */
void js_finalise(void)
{
}

/* exported interface documented in javascript/js.h */
nserror js_newheap(int timeout, jsheap **heap)
{
	struct jsheap *h;

	(void)timeout; /* \todo wire up a script execution watchdog */

	h = calloc(1, sizeof(*h));
	if (h == NULL) {
		return NSERROR_NOMEM;
	}

	h->rt = JS_NewRuntime();
	if (h->rt == NULL) {
		free(h);
		return NSERROR_NOMEM;
	}

	*heap = h;
	return NSERROR_OK;
}

/* exported interface documented in javascript/js.h */
void js_destroyheap(jsheap *heap)
{
	if (heap == NULL) {
		return;
	}
	JS_FreeRuntime(heap->rt);
	free(heap);
}

/* exported interface documented in javascript/js.h */
nserror js_newthread(jsheap *heap, void *win_priv, void *doc_priv, jsthread **thread)
{
	struct jsthread *t;

	t = calloc(1, sizeof(*t));
	if (t == NULL) {
		return NSERROR_NOMEM;
	}

	t->ctx = JS_NewContext(heap->rt);
	if (t->ctx == NULL) {
		free(t);
		return NSERROR_NOMEM;
	}

	/* \todo win_priv is a struct browser_window* by convention (see
	 * desktop/browser_window.c's CONTENT_MSG_GETTHREAD handler); once
	 * DOM bindings exist doc_priv (the html_content*) will back a real
	 * `document` global alongside `window`. */
	t->win = win_priv;
	t->doc_priv = doc_priv;
	t->closed = false;

	JS_SetContextOpaque(t->ctx, t);
	install_console(t->ctx);

	*thread = t;
	return NSERROR_OK;
}

/* exported interface documented in javascript/js.h */
nserror js_closethread(jsthread *thread)
{
	if (thread != NULL) {
		thread->closed = true;
	}
	return NSERROR_OK;
}

/* exported interface documented in javascript/js.h */
void js_destroythread(jsthread *thread)
{
	if (thread == NULL) {
		return;
	}
	JS_FreeContext(thread->ctx);
	free(thread);
}

/* exported interface documented in javascript/js.h */
bool js_exec(jsthread *thread, const uint8_t *txt, size_t txtlen, const char *name)
{
	JSValue result;
	bool ok = true;

	if (thread == NULL || thread->closed) {
		return false;
	}

	result = JS_Eval(thread->ctx, (const char *)txt, txtlen,
			name != NULL ? name : "<script>", JS_EVAL_TYPE_GLOBAL);

	if (JS_IsException(result)) {
		JSValue exc = JS_GetException(thread->ctx);
		const char *msg = JS_ToCString(thread->ctx, exc);

		if (thread->win != NULL) {
			browser_window_console_log(thread->win, BW_CS_SCRIPT_ERROR,
					msg, msg != NULL ? strlen(msg) : 0,
					BW_CS_FLAG_LEVEL_ERROR);
		} else {
			NSLOG(netsurf, ERROR, "script exception: %s", msg ? msg : "(?)");
		}

		if (msg != NULL) {
			JS_FreeCString(thread->ctx, msg);
		}
		JS_FreeValue(thread->ctx, exc);
		ok = false;
	}

	JS_FreeValue(thread->ctx, result);
	return ok;
}

/* exported interface documented in javascript/js.h */
bool js_fire_event(jsthread *thread, const char *type, struct dom_document *doc,
		struct dom_node *target)
{
	(void)thread;
	(void)type;
	(void)doc;
	(void)target;
	/* \todo needs the DOM/event binding layer */
	return true;
}

/* exported interface documented in javascript/js.h */
bool js_dom_event_add_listener(jsthread *thread, struct dom_document *document,
		struct dom_node *node, struct dom_string *event_type_dom, void *js_funcval)
{
	(void)thread;
	(void)document;
	(void)node;
	(void)event_type_dom;
	(void)js_funcval;
	/* \todo needs the DOM/event binding layer */
	return true;
}

/* exported interface documented in javascript/js.h */
void js_handle_new_element(jsthread *thread, struct dom_element *node)
{
	(void)thread;
	(void)node;
	/* \todo needs the DOM binding layer (scan for on* attributes) */
}

/* exported interface documented in javascript/js.h */
void js_event_cleanup(jsthread *thread, struct dom_event *evt)
{
	(void)thread;
	(void)evt;
	/* \todo needs the DOM/event binding layer */
}
