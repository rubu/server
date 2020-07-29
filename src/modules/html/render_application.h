#pragma once

#include <vector>

#pragma warning(push)
#pragma warning(disable: 4458)
#include <include/cef_app.h>
#include <include/cef_version.h>
#pragma warning(pop)

namespace caspar { namespace html {

class renderer_application
    : public CefApp
    , CefRenderProcessHandler
{
    std::vector<CefRefPtr<CefV8Context>> contexts_;
    const bool                           enable_gpu_;

  public:
    explicit renderer_application(const bool enable_gpu);

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

    void
    OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;

    void OnContextReleased(CefRefPtr<CefBrowser>   browser,
                           CefRefPtr<CefFrame>     frame,
                           CefRefPtr<CefV8Context> context) override;
    
    void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;

    void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser>        browser,
                                  CefProcessId                 source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

    IMPLEMENT_REFCOUNTING(renderer_application);
};

}}