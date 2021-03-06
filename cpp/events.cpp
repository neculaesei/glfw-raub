#include <cstdlib>
#include <sstream>

#include "platform.hpp"
#include "win-state.hpp"
#include "events.hpp"


using namespace v8;
using namespace node;
using namespace std;


namespace glfw {

const char typeKeyup[] = "keyup";
const char typeKeydown[] = "keydown";
const char typeMouseenter[] = "mouseenter";
const char typeMouseleave[] = "mouseleave";


void NAN_INLINE(_emit(GLFWwindow *window, int argc, V8_VAR_VAL argv[])) {
	
	WinState *state = reinterpret_cast<WinState*>(glfwGetWindowUserPointer(window));
	
	if (Nan::New(state->events)->Has(JS_STR("emit"))) {
		
		Nan::Callback callback(Nan::New(state->events)->Get(JS_STR("emit")).As<Function>());
		
		if ( ! callback.IsEmpty() ) {
			Nan::AsyncResource async("glfw::_emit()");
			callback.Call(argc, argv, &async);
		}
		
	}
	
}


void windowPosCB(GLFWwindow *window, int xpos, int ypos) { NAN_HS;
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	SET_PROP(evt, "type", JS_STR("window_pos"));
	SET_PROP(evt, "x", JS_INT(xpos));
	SET_PROP(evt, "y", JS_INT(ypos));
	
	V8_VAR_VAL argv[2] = { JS_STR("window_pos"), evt };
	_emit(window, 2, argv);
	
}


void windowSizeCB(GLFWwindow *window, int w, int h) { NAN_HS;
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	SET_PROP(evt, "type", JS_STR("resize"));
	SET_PROP(evt, "width", JS_INT(w));
	SET_PROP(evt, "height", JS_INT(h));
	
	V8_VAR_VAL argv[2] = { JS_STR("resize"), evt };
	_emit(window, 2, argv);
	
}


void windowFramebufferSizeCB(GLFWwindow *window, int w, int h) { NAN_HS;
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	SET_PROP(evt, "type", JS_STR("framebuffer_resize"));
	SET_PROP(evt, "width", JS_INT(w));
	SET_PROP(evt, "height", JS_INT(h));
	
	V8_VAR_VAL argv[2] = { JS_STR("framebuffer_resize"), evt };
	_emit(window, 2, argv);
	
}


void windowDropCB(GLFWwindow *window, int count, const char **paths) { NAN_HS;
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	SET_PROP(evt, "type", JS_STR("drop"));
	
	V8_VAR_ARR list = Nan::New<Array>(count);
	for (int i = 0; i < count; i++) {
		list->Set(i, JS_STR(paths[i]));
	}
	
	SET_PROP(evt, "list", list);
	
	V8_VAR_VAL argv[2] = { JS_STR("drop"), evt };
	_emit(window, 2, argv);
	
}


void windowCloseCB(GLFWwindow *window) { NAN_HS;
	
	V8_VAR_VAL argv[1] = { JS_STR("quit") };
	_emit(window, 1, argv);
	
}


void windowRefreshCB(GLFWwindow *window) { NAN_HS;
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	SET_PROP(evt, "type", JS_STR("refresh"));
	SET_PROP(evt, "window", JS_OFFS(reinterpret_cast<uint64_t>(window)));
	
	V8_VAR_VAL argv[2] = { JS_STR("refresh"), evt };
	_emit(window, 2, argv);
	
}


void windowIconifyCB(GLFWwindow *window, int iconified) { NAN_HS;
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	SET_PROP(evt, "type", JS_STR("iconified"));
	SET_PROP(evt, "iconified", JS_BOOL(iconified));
	
	V8_VAR_VAL argv[2] = { JS_STR("iconified"), evt };
	_emit(window, 2, argv);
	
}


void windowFocusCB(GLFWwindow *window, int focused) { NAN_HS;
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	SET_PROP(evt, "type", JS_STR("focused"));
	SET_PROP(evt, "focused", JS_BOOL(focused));
	
	V8_VAR_VAL argv[2] = { JS_STR("focused"), evt };
	_emit(window, 2, argv);
	
}


void fillKey(V8_VAR_OBJ &evt, int glfwKey, int scancode, int action, int mods) {
	
	const char *keyName = glfwGetKeyName(glfwKey, scancode);
	
	if (action == GLFW_REPEAT) {
		SET_PROP(evt, "repeat", JS_BOOL(true));
	}
	
	SET_PROP(evt, "altKey", JS_BOOL(mods & GLFW_MOD_ALT));
	SET_PROP(evt, "ctrlKey", JS_BOOL(mods & GLFW_MOD_CONTROL));
	SET_PROP(evt, "metaKey", JS_BOOL(mods & GLFW_MOD_SUPER));
	SET_PROP(evt, "shiftKey", JS_BOOL(mods & GLFW_MOD_SHIFT));
	
	
	if (keyName) {
		SET_PROP(evt, "code", JS_STR(keyName));
		SET_PROP(evt, "key", JS_STR(keyName));
	} else {
		SET_PROP(evt, "code", Nan::Null());
		SET_PROP(evt, "key", Nan::Null());
	}
	
	SET_PROP(evt, "which", JS_INT(glfwKey));
	
}


void keyCB(GLFWwindow *window, int glfwKey, int scancode, int action, int mods) { NAN_HS;
	
	const char *keyName = glfwGetKeyName(glfwKey, scancode);
	
	bool isAltPressed = mods & GLFW_MOD_ALT;
	bool isCtrlPressed = mods & GLFW_MOD_CONTROL;
	
	if (
		keyName && (action == GLFW_PRESS || action == GLFW_REPEAT) &&
		! (isAltPressed || isCtrlPressed)
	) {
		WinState *state = reinterpret_cast<WinState*>(glfwGetWindowUserPointer(window));
		state->pendingKey = glfwKey;
		state->pendingScan = scancode;
		state->pendingAction = action;
		state->pendingMods = mods;
		return;
	}
	
	const char *typeFound;
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		typeFound = typeKeydown;
	} else {
		typeFound = typeKeyup;
	}
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	fillKey(evt, glfwKey, scancode, action, mods);
	SET_PROP(evt, "charCode", JS_INT(0));
	SET_PROP(evt, "type", JS_STR(typeFound));
	
	V8_VAR_VAL argv[2] = { JS_STR(typeFound), evt };
	_emit(window, 2, argv);
	
}


void charCB(GLFWwindow* window, unsigned codepoint) { NAN_HS;
	
	WinState *state = reinterpret_cast<WinState*>(glfwGetWindowUserPointer(window));
	
	if ( ! state->pendingKey ) {
		return;
	}
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	fillKey(
		evt,
		state->pendingKey,
		state->pendingScan,
		state->pendingAction,
		state->pendingMods
	);
	SET_PROP(evt, "charCode", JS_INT(codepoint));
	SET_PROP(evt, "type", JS_STR(typeKeydown));
	
	state->pendingKey = 0;
	state->pendingScan = 0;
	state->pendingAction = 0;
	state->pendingMods = 0;
	
	V8_VAR_VAL argv[2] = { JS_STR(typeKeydown), evt };
	_emit(window, 2, argv);
	
}


void fillMouse(V8_VAR_OBJ &evt, GLFWwindow *window, int mods = -1) {
	
	WinState *state = reinterpret_cast<WinState*>(glfwGetWindowUserPointer(window));
	
	SET_PROP(evt, "clientX", JS_NUM(state->mouseX));
	SET_PROP(evt, "clientY", JS_NUM(state->mouseY));
	SET_PROP(evt, "pageX", JS_NUM(state->mouseX));
	SET_PROP(evt, "pageY", JS_NUM(state->mouseY));
	SET_PROP(evt, "x", JS_NUM(state->mouseX));
	SET_PROP(evt, "y", JS_NUM(state->mouseY));
	
	if (mods > -1) {
		SET_PROP(evt, "shiftKey", JS_BOOL(mods & GLFW_MOD_SHIFT));
		SET_PROP(evt, "ctrlKey", JS_BOOL(mods & GLFW_MOD_CONTROL));
		SET_PROP(evt, "altKey", JS_BOOL(mods & GLFW_MOD_ALT));
		SET_PROP(evt, "metaKey", JS_BOOL(mods & GLFW_MOD_SUPER));
	} else {
		SET_PROP(evt, "ctrlKey", JS_BOOL(
			glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS
		));
		SET_PROP(evt, "shiftKey", JS_BOOL(
			glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS
		));
		SET_PROP(evt, "altKey", JS_BOOL(
			glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS
		));
		SET_PROP(evt, "metaKey", JS_BOOL(
			glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS
		));
	}
	
}


void cursorPosCB(GLFWwindow* window, double x, double y) { NAN_HS;
	
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	
	if (x < 0 || x >= w || y < 0 || y >= h) {
		return;
	}
	
	WinState *state = reinterpret_cast<WinState*>(glfwGetWindowUserPointer(window));
	state->mouseX = static_cast<int>(x);
	state->mouseY = static_cast<int>(y);
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	fillMouse(evt, window);
	SET_PROP(evt, "type", JS_STR("mousemove"));
	
	V8_VAR_VAL argv[2] = { JS_STR("mousemove"), evt };
	_emit(window, 2, argv);
	
}


void cursorEnterCB(GLFWwindow* window, int entered) { NAN_HS;
	
	const char *typeFound;
	if (entered) {
		typeFound = typeMouseenter;
	} else {
		typeFound = typeMouseleave;
	}
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	fillMouse(evt, window);
	SET_PROP(evt, "type", JS_STR(typeFound));
	
	V8_VAR_VAL argv[2] = { JS_STR(typeFound), evt };
	_emit(window, 2, argv);
	
}


void mouseButtonCB(GLFWwindow *window, int button, int action, int mods) { NAN_HS;
	
	V8_VAR_OBJ evt1 = Nan::New<Object>();
	fillMouse(evt1, window);
	SET_PROP(evt1, "type", JS_STR(action ? "mousedown" : "mouseup"));
	SET_PROP(evt1, "button", JS_INT(button));
	SET_PROP(evt1, "which", JS_INT(button));
	
	V8_VAR_VAL argv[2] = { JS_STR(action ? "mousedown" : "mouseup"), evt1 };
	_emit(window, 2, argv);
	
	if ( ! action ) {
		
		V8_VAR_OBJ evt2 = Nan::New<Object>();
		fillMouse(evt2, window);
		SET_PROP(evt2, "type", JS_STR("click"));
		SET_PROP(evt2, "button", JS_INT(button));
		SET_PROP(evt2, "which", JS_INT(button));
		
		V8_VAR_VAL argv[2] = { JS_STR("click"), evt2 };
		_emit(window, 2, argv);
		
	}
	
}


void scrollCB(GLFWwindow *window, double xoffset, double yoffset) { NAN_HS;
	
	double dx = xoffset * 100;
	double dy = yoffset * 100;
	
	V8_VAR_OBJ evt = Nan::New<Object>();
	fillMouse(evt, window);
	SET_PROP(evt, "type", JS_STR("wheel"));
	SET_PROP(evt, "wheelDeltaX", JS_NUM(dx));
	SET_PROP(evt, "wheelDeltaY", JS_NUM(dy));
	SET_PROP(evt, "wheelDelta", JS_NUM(dy));
	
	V8_VAR_VAL argv[2] = { JS_STR("wheel"), evt };
	_emit(window, 2, argv);
	
}


} // namespace glfw
