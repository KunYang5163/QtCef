#include "qtcefapp.h"
#include "cef_app.h"
#include "cef_parser.h"
#include "cef_browser.h"
#include "cef_command_line.h"
#include "cef_browser.h"
#include "wrapper/cef_closure_task.h"
#include "views/cef_browser_view_delegate.h"
#include "views/cef_view.h"
#include "views/cef_browser_view.h"
#include "views/cef_window.h"
#include <QString>
#include <QWidget>
#include <string>
#include <QTimer>
#include <QMouseEvent>
#include <QApplication>
#include <QTest>

namespace {

QtCefHandler* g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type)
{
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

static RECT qRectToRECT(const QRect& rect)
{
    RECT r;
    r.top = rect.top();
    r.bottom = rect.bottom();
    r.left = rect.left();
    r.right = rect.right();
    return r;
}

static inline int MiddleX(const CefRect& rect) {
    return rect.x + rect.width / 2;
}

static inline int MiddleY(const CefRect& rect) {
    return rect.y + rect.height / 2;
}

}  // namespace

QtCefHandler::QtCefHandler(CefRefPtr<QtCefApp> cefApp)
    : _cefApp(cefApp), is_closing_(false)
{
    DCHECK(!g_instance);
    g_instance = this;
}

QtCefHandler::~QtCefHandler()
{
    g_instance = nullptr;
}

// static
QtCefHandler* QtCefHandler::GetInstance()
{
    return g_instance;
}

void QtCefHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    // Add to the list of existing browsers.
    browser_list_.push_back(browser);
}

bool QtCefHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
    Q_UNUSED(browser);
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed destription of this
    // process.
    if (browser_list_.size() == 1) {
      // Set a flag to indicate that the window close should be allowed.
      is_closing_ = true;
    }

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void QtCefHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Remove from the list of existing browsers.
    BrowserList::iterator bit = browser_list_.begin();
    for (; bit != browser_list_.end(); ++bit)
    {
        if ((*bit)->IsSame(browser))
        {
            browser_list_.erase(bit);
            break;
        }
    }

    if (browser_list_.empty())
    {

    }
}

void QtCefHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    Q_UNUSED(browser);
    CEF_REQUIRE_UI_THREAD();

    qDebug() << "CEF ERROR (" << errorCode << "): " << errorText.ToWString();

    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED)
      return;

    // Display a load error message using a data: URI.
    std::stringstream ss;
    ss << "<html><body bgcolor=\"white\">"
          "<h2>Failed to load URL "
       << std::string(failedUrl) << " with error " << std::string(errorText)
       << " (" << errorCode << ").</h2></body></html>";

    frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void QtCefHandler::login(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    // browser.find_element(By.XPATH,'//*[@id="app"]/div/div/div/section/form/div[1]/div/div/input').send_keys(username)
    // browser.find_element(By.XPATH,'//*[@id="app"]/div/div/div/section/form/div[2]/div/div/input').send_keys(passwd)
    // browser.find_element(By.XPATH,'//*[@id="app"]/div/div/div/section/section/button').click()
}

void QtCefHandler::checkUrl(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,const CefString &url)
{
    qDebug() << "check url: "  << url.ToWString();
    if (url.ToString().find("lhqkl.ydmap.cn/user/login") != std::string::npos)
    {
        // CefString js = "";
                       // (function() {
                       // browser.find_element(By.XPATH,'//*[@id=\"app\"]/div/div/div/section/form/div[1]/div/div/input').send_keys("15118025057")
                       // browser.find_element(By.XPATH,'//*[@id=\"app\"]/div/div/div/section/form/div[2]/div/div/input').send_keys("topten10")
                       // browser.find_element(By.XPATH,'//*[@id=\"app\"]/div/div/div/section/section/button').click()
                       // })()
                       // ";
        // browser->GetMainFrame()->ExecuteJavaScript(js, nullptr, 0);
        CefRefPtr<CefFrame> frame = browser->GetMainFrame();
        // frame->ExecuteJavaScript("alert('ExecuteJavaScript works!');",
                                 // frame->GetURL(), 0);
        // frame->ExecuteJavaScript(""
        //                " var username = getElementByXpath(By.XPATH,'//*[@id=\"app\"]/div/div/div/section/form/div[1]/div/div/input');"
        //                " username.send_keys(\"15118025057\");"
        //                " var pwd = getElementByXpath(By.XPATH,'//*[@id=\"app\"]/div/div/div/section/form/div[2]/div/div/input');"
        //                " pwd.send_keys(\"topten10\");"
        //                " var login = getElementByXpath(By.XPATH,'//*[@id=\"app\"]/div/div/div/section/section/button');"
        //                " login.click();"
        //                          "",
        //                          frame->GetURL(), 0);

        // browser->GetHost()->Find(0, "请输入您的手机号", true, true, false);
    }
    // if (checkUrl.currentUrl === 'https://lhqkl.ydmap.cn/booking/schedule/103909?salesItemId=102914')
    // {
    //     selectCourt();
    // }
    // else if (checkUrl.currentUrl.includes('contact-collect'))
    // {
    //     confirmMember();
    // }
    // else if (checkUrl.currentUrl.includes('dataKey='))
    // {
    //     confirmCourt();
    // }
}

void QtCefHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   const CefString& url)
{
    qDebug() << "address change: " << url.ToString();
}

void QtCefHandler::OnFindResult(CefRefPtr<CefBrowser> browser,
                          int identifier,
                          int count,
                          const CefRect& selectionRect,
                          int activeMatchOrdinal,
                          bool finalUpdate)
{
    if (count == 0)
    {
        return ;
    }
    qDebug() << "Find count: " << count;
    // QRect selected = QRect(selectionRect.x, selectionRect.y, selectionRect.width, selectionRect.height);
    // qDebug() << "select rect: " << selected;
    qDebug() << "click point: " << MiddleX(selectionRect) << ", " << MiddleY(selectionRect);

    if (selectionRect.width == 0 || selectionRect.height == 0)
    {
        return ;
    }
    CefMouseEvent event;
    event.x = MiddleX(selectionRect);
    event.y = MiddleY(selectionRect);
    qDebug() << "event point: " << event.x << "," << event.y;
    // browser->GetHost()->SendMouseMoveEvent(event, true);
    browser->GetHost()->SendMouseMoveEvent(event, true);
    browser->GetHost()->SendMouseClickEvent(event, MBT_LEFT, true, 1);

    // QWidget *d = qApp->activeWindow();
    // QTest::mouseClick(d, Qt::LeftButton, Qt::NoModifier, selected.center());

    // QCursor::setPos(selected.center());
    // qDebug()<<QCursor::pos();
    // QWidget *d = qApp->activeWindow();
    // QMouseEvent MouseEvent(QEvent::MouseButtonPress, QCursor::pos(),Qt::LeftButton,Qt::LeftButton, Qt::NoModifier);
    // QApplication::sendEvent(d, &MouseEvent);
}

void QtCefHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                       CefRefPtr<CefFrame> frame,
                       int httpStatusCode)
{
    auto url =  browser->GetMainFrame()->GetURL();
    qDebug() << "OnLoadEnd: " << url.ToWString();
    checkUrl(browser, frame, url);
}

CefResponseFilter::FilterStatus ResponseDataInterceptor::Filter(void *data_in, size_t data_in_size, size_t &data_in_read, void *data_out, size_t data_out_size, size_t &data_out_written)
{
    CEF_REQUIRE_IO_THREAD();
    DCHECK((data_in_size == 0U && !data_in) || (data_in_size > 0U && data_in));
    DCHECK_EQ(data_in_read, 0U);
    DCHECK(data_out);
    DCHECK_GT(data_out_size, 0U);
    DCHECK_EQ(data_out_written, 0U);

    if (data_in == nullptr || data_in_size == 0)
    {
        data_in_read = 0;
        data_out_written = 0;
        return RESPONSE_FILTER_DONE;
    }

    data_in_read = data_in_size;

    if (data_in_read > data_out_size)
    {
        data_out_written = data_out_size;
    }
    else
    {
        data_out_written = data_in_read;
    }

    if (data_out_written > 0)
    {
        memcpy(data_out, data_in, data_out_written);
    }

    _interceptedData->append((const char*)data_out, (int)data_out_written);

    if (data_in_read < data_out_size)
    {
        return RESPONSE_FILTER_NEED_MORE_DATA;
    }
    else
    {
        return RESPONSE_FILTER_DONE;
    }
}

void QtCefHandler::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response, CefResourceRequestHandler::URLRequestStatus status, int64 received_content_length)
{
    CEF_REQUIRE_IO_THREAD();
    Q_UNUSED(browser);
    Q_UNUSED(frame);
    Q_UNUSED(response);
    Q_UNUSED(status);
    Q_UNUSED(received_content_length);

    const uint64& identifier = request->GetIdentifier();

    if (identifier == _interceptor->GetResponseIdentifier())
    {
        _cefApp->OnDataReceived(_interceptor->GetInterceptedData());
    }
    qDebug() << __func__;
    browser->GetHost()->Find(0, "请输入您的手机号", true, true, false);
}

void QtCefHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::Bind(&QtCefHandler::CloseAllBrowsers, this,
                                   force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
      (*it)->GetHost()->CloseBrowser(force_close);
}

QtCefApp::~QtCefApp()
{
    //if (_browser && _browser->GetHost())
    //{
    //    _browser->GetHost()->CloseBrowser(true);
    //}
}

void QtCefApp::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();

    _cefHandler = new QtCefHandler(this);
}

CefWindowHandle QtCefApp::CreatBrowserWindow(QWidget* parentWidget)
{
    CefWindowInfo window_info;

#if defined(OS_WIN)
    if (parentWidget)
    {
        window_info.SetAsChild((CefWindowHandle)parentWidget->winId(), qRectToRECT(parentWidget->rect()));
    }
    else
    {
        window_info.SetAsPopup(nullptr, "Browser");
    }

#endif
    _browser = CefBrowserHost::CreateBrowserSync(window_info, _cefHandler, "about:blank", CefBrowserSettings(), nullptr, nullptr);

    return _browser->GetHost()->GetWindowHandle();
}

void QtCefApp::OnDataReceived(std::shared_ptr<QByteArray> data)
{
    emit dataReceived(data);
}

void QtCefApp::setProxyServer(const QString &address, int port)
{
    _proxyServerAddress = address;
    _proxyServerPort = port;
    if (_proxyEnabled)
    {
        updateProxySettings();
    }
}

void QtCefApp::setProxyEnabled(bool enabled)
{
    _proxyEnabled = enabled;
    updateProxySettings();
}

void QtCefApp::updateProxySettings()
{
    if (_browser)
    {
        if (_proxyEnabled)
        {
            auto _host = _browser->GetHost();
            auto requestContext = _host->GetRequestContext();
            CefString error;
            auto proxyDictionary = CefDictionaryValue::Create();
            proxyDictionary->SetString(L"mode", L"fixed_servers");
            proxyDictionary->SetString(L"server", QString("%1:%2").arg(_proxyServerAddress).arg(_proxyServerPort).toStdWString());
            auto proxyValue = CefValue::Create();
            proxyValue->SetDictionary(proxyDictionary);
            if (!requestContext->SetPreference(L"proxy", proxyValue, error))
            {
                qWarning() << Q_FUNC_INFO << ": Error while enable proxy: " << error.c_str();
            }
        }
        else
        {
            auto _host = _browser->GetHost();
            auto requestContext = _host->GetRequestContext();
            CefString error;
            auto proxyDictionary = CefDictionaryValue::Create();
            proxyDictionary->SetString(L"no-proxy-server", L"1");
            auto proxyValue = CefValue::Create();
            proxyValue->SetDictionary(proxyDictionary);
            if (!requestContext->SetPreference(L"proxy", proxyValue, error))
            {
                qWarning() << Q_FUNC_INFO << ": Error while disable proxy: " << error.c_str();
            }
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << ": _browser == nullptr";
    }

    reloadUrl();
}

void QtCefApp::reloadUrl()
{
    if (_cefHandler)
    {
        _cefHandler->StopIntercepting();
    }

    if (_browser)
    {
        _browser->GetMainFrame()->LoadURL("");
        _browser->GetMainFrame()->LoadURL(_url);
    }
    else
    {
        qWarning() << Q_FUNC_INFO << ": _browser == nullptr";
    }
}
