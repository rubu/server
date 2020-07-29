#include <include/cef_app.h>
#include <modules/html/render_application.h>

int main(int argc, char** argv) 
{
  CefMainArgs main_args(argc, argv);
  CefRefPtr<caspar::html::renderer_application> app(new caspar::html::renderer_application(false));
  return CefExecuteProcess(main_args, app.get(), nullptr);
}
