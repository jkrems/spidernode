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

#include <assert.h>

#include "tty_wrap.h"

namespace knot {

static JSClass TTYWrap_class = {
  "TTY",  /* name */
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

static JSFunctionSpec TTYWrap_methods[] = {
  JS_FN("write", TTYWrap::Write, 1, 0),
  JS_FN("unref", TTYWrap::Unref, 0, 0),
  JS_FS_END
};

void TTYWrap::Initialize(JSContext *cx, JS::HandleObject exports) {

  JS_InitClass(cx, exports,
    js::NullPtr(), /* parent_proto */
    &TTYWrap_class, /* clasp */
    TTYWrap::New, /* constructor */
    2, /* nargs */
    NULL, /* properties */
    TTYWrap_methods, /* methods */
    NULL, /* static properties */
    NULL /* static methods */
  );

  JS_DefineFunction(cx, exports, "guessHandleType", &GuessHandleType, 1, 0);

  // Environment* env = Environment::GetCurrent(context);

  // Local<FunctionTemplate> t = FunctionTemplate::New(New);
  // t->SetClassName(FIXED_ONE_BYTE_STRING(node_isolate, "TTY"));
  // t->InstanceTemplate()->SetInternalFieldCount(1);

  // enum PropertyAttribute attributes =
  //     static_cast<PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
  // t->InstanceTemplate()->SetAccessor(FIXED_ONE_BYTE_STRING(node_isolate, "fd"),
  //                                    StreamWrap::GetFD,
  //                                    NULL,
  //                                    Handle<Value>(),
  //                                    v8::DEFAULT,
  //                                    attributes);

  // NODE_SET_PROTOTYPE_METHOD(t, "close", HandleWrap::Close);
  // NODE_SET_PROTOTYPE_METHOD(t, "unref", HandleWrap::Unref);

  // NODE_SET_PROTOTYPE_METHOD(t, "readStart", StreamWrap::ReadStart);
  // NODE_SET_PROTOTYPE_METHOD(t, "readStop", StreamWrap::ReadStop);

  // NODE_SET_PROTOTYPE_METHOD(t, "writeBuffer", StreamWrap::WriteBuffer);
  // NODE_SET_PROTOTYPE_METHOD(t,
  //                           "writeAsciiString",
  //                           StreamWrap::WriteAsciiString);
  // NODE_SET_PROTOTYPE_METHOD(t, "writeUtf8String", StreamWrap::WriteUtf8String);
  // NODE_SET_PROTOTYPE_METHOD(t, "writeUcs2String", StreamWrap::WriteUcs2String);

  // NODE_SET_PROTOTYPE_METHOD(t, "getWindowSize", TTYWrap::GetWindowSize);
  // NODE_SET_PROTOTYPE_METHOD(t, "setRawMode", SetRawMode);

  // NODE_SET_METHOD(target, "isTTY", IsTTY);
  // NODE_SET_METHOD(target, "guessHandleType", GuessHandleType);

  // target->Set(FIXED_ONE_BYTE_STRING(node_isolate, "TTY"), t->GetFunction());
  // env->set_tty_constructor_template(t);
}


uv_tty_t* TTYWrap::UVHandle() {
  return &handle_;
}


bool TTYWrap::Unref(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  JS::RootedObject self(cx, &args.thisv().toObject());
  TTYWrap *wrap = (TTYWrap *) JS_GetPrivate(self);

  uv_unref((uv_handle_t *)wrap->UVHandle());

  return true;
}


bool TTYWrap::GuessHandleType(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  int fd = args[0].toInt32();
  assert(fd >= 0);

  uv_handle_type t = uv_guess_handle(fd);
  const char* type = NULL;

  switch (t) {
  case UV_TCP: type = "TCP"; break;
  case UV_TTY: type = "TTY"; break;
  case UV_UDP: type = "UDP"; break;
  case UV_FILE: type = "FILE"; break;
  case UV_NAMED_PIPE: type = "PIPE"; break;
  case UV_UNKNOWN_HANDLE: type = "UNKNOWN"; break;
  default:
    abort();
  }

  args.rval().setString(JS_NewStringCopyZ(cx, type));
  return true;
}


bool TTYWrap::IsTTY(JSContext *cx, unsigned argc, JS::Value *vp) {
  // HandleScope scope(node_isolate);
  // int fd = args[0]->Int32Value();
  // assert(fd >= 0);
  // bool rc = uv_guess_handle(fd) == UV_TTY;
  // args.GetReturnValue().Set(rc);
  return true;
}


bool TTYWrap::GetWindowSize(JSContext *cx, unsigned argc, JS::Value *vp) {
  // HandleScope scope(node_isolate);

  // TTYWrap* wrap = Unwrap<TTYWrap>(args.This());
  // assert(args[0]->IsArray());

  // int width, height;
  // int err = uv_tty_get_winsize(&wrap->handle_, &width, &height);

  // if (err == 0) {
  //   Local<v8::Array> a = args[0].As<Array>();
  //   a->Set(0, Integer::New(width, node_isolate));
  //   a->Set(1, Integer::New(height, node_isolate));
  // }

  // args.GetReturnValue().Set(err);
  return true;
}


bool TTYWrap::SetRawMode(JSContext *cx, unsigned argc, JS::Value *vp) {
  // HandleScope scope(node_isolate);

  // TTYWrap* wrap = Unwrap<TTYWrap>(args.This());

  // int err = uv_tty_set_mode(&wrap->handle_, args[0]->IsTrue());
  // args.GetReturnValue().Set(err);
  return true;
}


bool TTYWrap::Write(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  JSAutoByteString chunk(cx, args[0].toString());

  JS::RootedObject self(cx, &args.thisv().toObject());
  TTYWrap *wrap = (TTYWrap *) JS_GetPrivate(self);

  WriteWrap *write = new WriteWrap();
  uv_write_t &req = write->req;
  uv_buf_t &buf = write->buf;
  buf.base = (char*)chunk.ptr();
  buf.len = strlen(buf.base);
  uv_write(&req, (uv_stream_t*) wrap->UVHandle(), &buf, 1, TTYWrap::AfterWrite);

  return true;
}


void TTYWrap::AfterWrite(uv_write_t *req, int status) {
  WriteWrap *write = (WriteWrap*)req;
  delete write;
}


bool TTYWrap::New(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  JS::RootedObject tty(cx, JS_NewObjectForConstructor(cx, &TTYWrap_class, vp));
  if (!tty)
    return false;

  int32_t fd = args[0].toInt32();
  bool readable = args[1].toBoolean();

  TTYWrap *wrap = new TTYWrap(cx, fd, readable);
  JS_SetPrivate(tty, wrap);

  args.rval().setObject(*tty);

  return true;
}


TTYWrap::TTYWrap(JSContext *cx, int fd, bool readable) {
  uv_loop_t *loop = (uv_loop_t *)JS_GetContextPrivate(cx);

  uv_tty_init(loop, &handle_, fd, readable);
  handle_.data = this;
}

}  // namespace knot
