// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "timer_wrap.h"

#include <stdint.h>
#include <assert.h>

using JS::DoubleValue;
using JS::Int32Value;

namespace knot {

static JSClass TimerWrap_class = {
  "Timer",  /* name */
  JSCLASS_HAS_PRIVATE,  /* flags */
  JS_PropertyStub, /* addProperty */
  JS_DeletePropertyStub, /* delProperty */
  JS_PropertyStub, /* getProperty */
  JS_StrictPropertyStub, /* setProperty */
  JS_EnumerateStub, /* enumerate */
  JS_ResolveStub, /* resolve */
  JS_ConvertStub, /* convert */
  0, /* finalize */
  0, /* JSNative call */
  0, /* JSHasInstanceOp hasInstance */
  0, /* JSNative construct */
  0 /* JSTraceOp trace */
  /* JSCLASS_NO_OPTIONAL_MEMBERS */
};

static JSFunctionSpec TimerWrap_methods[] = {
  JS_FN("start", TimerWrap::Start, 2, 0),
  JS_FS_END
};

static JSFunctionSpec TimerWrap_staticMethods[] = {
  JS_FN("now", TimerWrap::Now, 0, 0),
  JS_FS_END
};

const uint32_t kOnTimeout = 0;

void TimerWrap::Initialize(JSContext *cx, JS::HandleObject exports) {

  JS_InitClass(cx, exports,
    js::NullPtr(), /* parent_proto */
    &TimerWrap_class, /* clasp */
    TimerWrap::New, /* constructor */
    2, /* nargs */
    NULL, /* properties */
    TimerWrap_methods, /* methods */
    NULL, /* static properties */
    TimerWrap_staticMethods /* static methods */
  );

  // Local<FunctionTemplate> constructor = FunctionTemplate::New(New);
  // constructor->InstanceTemplate()->SetInternalFieldCount(1);
  // constructor->SetClassName(FIXED_ONE_BYTE_STRING(node_isolate, "Timer"));
  // constructor->Set(FIXED_ONE_BYTE_STRING(node_isolate, "kOnTimeout"),
  //                  Integer::New(kOnTimeout, node_isolate));

  // NODE_SET_METHOD(constructor, "now", Now);

  // NODE_SET_PROTOTYPE_METHOD(constructor, "close", HandleWrap::Close);
  // NODE_SET_PROTOTYPE_METHOD(constructor, "ref", HandleWrap::Ref);
  // NODE_SET_PROTOTYPE_METHOD(constructor, "unref", HandleWrap::Unref);

  // NODE_SET_PROTOTYPE_METHOD(constructor, "start", Start);
  // NODE_SET_PROTOTYPE_METHOD(constructor, "stop", Stop);
  // NODE_SET_PROTOTYPE_METHOD(constructor, "setRepeat", SetRepeat);
  // NODE_SET_PROTOTYPE_METHOD(constructor, "getRepeat", GetRepeat);
  // NODE_SET_PROTOTYPE_METHOD(constructor, "again", Again);

  // target->Set(FIXED_ONE_BYTE_STRING(node_isolate, "Timer"),
  //             constructor->GetFunction());
}

bool TimerWrap::New(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  JS::RootedObject timer(cx, JS_NewObjectForConstructor(cx, &TimerWrap_class, vp));
  if (!timer)
    return false;

  TimerWrap *wrap = new TimerWrap(cx, timer);
  JS_SetPrivate(timer, wrap);

  args.rval().setObject(*timer);

  return true;
}

bool TimerWrap::Start(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  JS::RootedObject self(cx, &args.thisv().toObject());
  TimerWrap *wrap = (TimerWrap *) JS_GetPrivate(self);

  int64_t timeout = args[0].toInt32();
  int64_t repeat = args[1].toInt32();
  int err = uv_timer_start(&wrap->handle_, TimerWrap::OnTimeout, timeout, repeat);
  args.rval().set(Int32Value(err));
  return true;
}

bool TimerWrap::Stop(JSContext *cx, unsigned argc, JS::Value *vp) {
  // HandleScope scope(node_isolate);
  // TimerWrap* wrap = Unwrap<TimerWrap>(args.This());

  // int err = uv_timer_stop(&wrap->handle_);
  // args.GetReturnValue().Set(err);
  return true;
}

bool TimerWrap::Again(JSContext *cx, unsigned argc, JS::Value *vp) {
  // HandleScope scope(node_isolate);
  // TimerWrap* wrap = Unwrap<TimerWrap>(args.This());

  // int err = uv_timer_again(&wrap->handle_);
  // args.GetReturnValue().Set(err);
  return true;
}

bool TimerWrap::SetRepeat(JSContext *cx, unsigned argc, JS::Value *vp) {
  // HandleScope scope(node_isolate);
  // TimerWrap* wrap = Unwrap<TimerWrap>(args.This());

  // int64_t repeat = args[0]->IntegerValue();
  // uv_timer_set_repeat(&wrap->handle_, repeat);
  // args.GetReturnValue().Set(0);
  return true;
}

bool TimerWrap::GetRepeat(JSContext *cx, unsigned argc, JS::Value *vp) {
  // HandleScope scope(node_isolate);
  // TimerWrap* wrap = Unwrap<TimerWrap>(args.This());

  // int64_t repeat = uv_timer_get_repeat(&wrap->handle_);
  // args.GetReturnValue().Set(static_cast<double>(repeat));
  return true;
}

void TimerWrap::OnTimeout(uv_timer_t* handle) {
  TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
  wrap->MakeCallback(kOnTimeout, JS::HandleValueArray::empty());
}

bool TimerWrap::Now(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  uv_update_time(uv_default_loop());
  double now = static_cast<double>(uv_now(uv_default_loop()));
  
  args.rval().set(DoubleValue(now));
  return true;
}

TimerWrap::TimerWrap(JSContext *cx, JS::HandleObject object) :
    context_(cx), object_(cx, object) {
  int r = uv_timer_init(uv_default_loop(), &handle_);
  assert(r == 0);
  handle_.data = this;
}

TimerWrap::~TimerWrap() {
}


JS::Value* TimerWrap::MakeCallback(JS::HandleValue cb,
                                   const JS::HandleValueArray& argv) {

  JS::RootedValue rval(context_);

  printf("About to call callback: %d\n", cb.isObject());
  if (!JS_CallFunctionValue(context_, object_, cb, argv, &rval)) {
    printf("Failed to call timeout callback\n");
    return NULL;
  }
  printf("Called callback\n");
  // assert(env()->context() == env()->isolate()->GetCurrentContext());

  // v8::Local<v8::Object> context = object();
  // v8::Local<v8::Object> process = env()->process_object();

  // v8::TryCatch try_catch;
  // try_catch.SetVerbose(true);

  // if (has_async_queue()) {
  //   v8::Local<v8::Value> val = context.As<v8::Value>();
  //   env()->async_listener_load_function()->Call(process, 1, &val);

  //   if (try_catch.HasCaught())
  //     return v8::Undefined(env()->isolate());
  // }

  // v8::Local<v8::Value> ret = cb->Call(context, argc, argv);

  // if (try_catch.HasCaught()) {
  //   return Undefined(env()->isolate());
  // }

  // if (has_async_queue()) {
  //   v8::Local<v8::Value> val = context.As<v8::Value>();
  //   env()->async_listener_unload_function()->Call(process, 1, &val);

  //   if (try_catch.HasCaught())
  //     return v8::Undefined(env()->isolate());
  // }

  // Environment::TickInfo* tick_info = env()->tick_info();

  // if (tick_info->in_tick()) {
  //   return ret;
  // }

  // if (tick_info->length() == 0) {
  //   tick_info->set_index(0);
  //   return ret;
  // }

  // tick_info->set_in_tick(true);

  // // TODO(trevnorris): Consider passing "context" to _tickCallback so it
  // // can then be passed as the first argument to the nextTick callback.
  // // That should greatly help needing to create closures.
  // env()->tick_callback_function()->Call(process, 0, NULL);

  // tick_info->set_in_tick(false);

  // if (try_catch.HasCaught()) {
  //   tick_info->set_last_threw(true);
  //   return Undefined(env()->isolate());
  // }

  // return ret;
  return NULL;
}


// v8::Handle<v8::Value> AsyncWrap::MakeCallback(
//     const v8::Handle<v8::String> symbol,
//     int argc,
//     v8::Handle<v8::Value>* argv) {
//   v8::Local<v8::Value> cb_v = object()->Get(symbol);
//   v8::Local<v8::Function> cb = cb_v.As<v8::Function>();
//   assert(cb->IsFunction());

//   return MakeCallback(cb, argc, argv);
// }


JS::Value* TimerWrap::MakeCallback(uint32_t index,
                                   const JS::HandleValueArray& argv) {
  JS::RootedValue cb_v(context_);
  if (!JS_GetElement(context_, object_, index, &cb_v)) {
    printf("Could not get property #%d\n", index);
    return NULL;
  }
  return MakeCallback(cb_v, argv);
  // v8::Local<v8::Value> cb_v = object()->Get(index);
  // v8::Local<v8::Function> cb = cb_v.As<v8::Function>();
  // assert(cb->IsFunction());

  // return MakeCallback(cb, argc, argv);
}


}  // namespace knot
