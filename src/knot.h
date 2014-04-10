#ifndef SRC_KNOT_H_
#define SRC_KNOT_H_

#include <jsapi.h>
#include <uv.h>

#include <stdint.h>
#include <cstddef>

#define KNOT_DEFAULT_MAX_BYTES 8L * 1024 * 1024

namespace knot {

class Knot {
  public:
    Knot();
    ~Knot();

    bool init(uint32_t maxBytes = KNOT_DEFAULT_MAX_BYTES,
              size_t stackChunkSize = 8192);

    int runScriptFile(const char *filename);

  private:
    JSRuntime *runtime;
    JSContext *context;
    JS::Heap<JSObject*> global;

    uv_loop_t *loop;

    bool bootstrapCore();

    int _executeScriptFile(const char *filename);
    int _executeScript(const char *filename, const char* source, size_t length);

    JSObject* NewGlobalObject();

    static void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report);
    static bool Binding(JSContext *cx, unsigned argc, JS::Value *vp);
};

}

#endif  // SRC_KNOT_H_
