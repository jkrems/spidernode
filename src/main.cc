#include "knot.h"

int main(int argc, const char *argv[])
{
    using knot::Knot;

    Knot knot;

    if (!knot.init()) {
      return 1;
    }

    return knot.runScriptFile(argv[1]);
    // JSRuntime *rt;
    // JSContext *cx;

    // JS_Init();

    // rt = JS_NewRuntime(8L * 1024 * 1024, JS_NO_HELPER_THREADS);
    // if (rt == NULL)
    //     return 1;

    // cx = JS_NewContext(rt, 8192);
    // if (cx == NULL)
    //     return 1;

    // JS_SetErrorReporter(cx, jsReporter);

    // int exitCode = run(cx, argv[1]);

    // JS_DestroyContext(cx);
    // JS_DestroyRuntime(rt);
    // JS_ShutDown();

    // return exitCode;
}
