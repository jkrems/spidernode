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

#ifndef SRC_TTY_WRAP_H_
#define SRC_TTY_WRAP_H_

#include <jsapi.h>
#include <uv.h>

namespace knot {

class TTYWrap {
  public:
    static void Initialize(JSContext *cx, JS::HandleObject exports);

    uv_tty_t* UVHandle();

    static bool GuessHandleType(JSContext *cx, unsigned argc, JS::Value *vp);
    static bool IsTTY(JSContext *cx, unsigned argc, JS::Value *vp);
    static bool GetWindowSize(JSContext *cx, unsigned argc, JS::Value *vp);
    static bool SetRawMode(JSContext *cx, unsigned argc, JS::Value *vp);
    static bool Write(JSContext *cx, unsigned argc, JS::Value *vp);
    static bool New(JSContext *cx, unsigned argc, JS::Value *vp);

  private:
    TTYWrap(int fd,
            bool readable);

    uv_tty_t handle_;
};

}  // namespace knot

#endif  // SRC_TTY_WRAP_H_
