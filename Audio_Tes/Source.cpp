#include <Windows.h>
#include <audiopolicy.h>
#include <Audioclient.h>
#include <Mmdeviceapi.h>

#define CHECK(call) if((hr = call) < 0) { handle_error(hr, __LINE__); return -1; }

int format_int_dec(WCHAR* buf, int n)
{
    int k = n;

    int c = 1;

    while (k >= 10)
    {
        k /= 10;
        ++c;
    }

    int chars = c;

    while (n)
    {
        buf[--c] = L'0' + (n % 10);
        n /= 10;
    }

    return chars;
}

int format_int_hex(WCHAR* buf, int n)
{
    int k = n;

    int c = 1;

    while (k >= 16)
    {
        k /= 16;
        ++c;
    }

    int chars = c;

    while (n)
    {
        int digit = (n % 16);

        if (digit < 10)
            buf[--c] = L'0' + digit;
        else
            buf[--c] = L'A' + digit - 10;

        n /= 16;
    }

    return chars;
}

__declspec(noinline) void handle_error(HRESULT hr, int line)
{
    WCHAR msg_buf[256]{};
    
    const WCHAR* msg_fmt_1 = L"Encountered an Error on line ";
    const WCHAR* msg_fmt_2 = L". Error-Code: 0x";
    
    int idx = 0;
    
    for (int i = 0; msg_fmt_1[i] != L'\0'; ++i)
        msg_buf[idx++] = msg_fmt_1[i];
    
    idx += format_int_dec(msg_buf + idx, line);

    for (int i = 0; msg_fmt_2[i] != L'\0'; ++i)
        msg_buf[idx++] = msg_fmt_2[i];
    
    format_int_hex(msg_buf + idx, hr);

    MessageBox(nullptr, msg_buf, nullptr, MB_OK);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,int nShowCmd)
{
    int line_no = 0;

    HRESULT hr = S_OK;

    CHECK(CoInitialize(nullptr));

    IMMDeviceEnumerator* device_enumerator = nullptr;

    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    CHECK(CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&device_enumerator));

    IMMDevice* endpoint = nullptr;

    CHECK(device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &endpoint));
    
    IAudioSessionManager2* session_manager_2 = nullptr;

    CHECK(endpoint->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, nullptr, reinterpret_cast<void**>(&session_manager_2)));

    IAudioSessionEnumerator* session_enumerator = nullptr;

    CHECK(session_manager_2->GetSessionEnumerator(&session_enumerator));

    int session_cnt;
        
    CHECK(session_enumerator->GetCount(&session_cnt));

    for (int i = 0; i != session_cnt; ++i)
    {
        IAudioSessionControl* session_control;

        CHECK(session_enumerator->GetSession(i, &session_control));

        ISimpleAudioVolume* volume;
        
        CHECK(session_control->QueryInterface(__uuidof(volume), (void**)&volume));

        CHECK(volume->SetMasterVolume(1.0F, nullptr));
    }

#ifdef _DEBUG

    MessageBox(nullptr, L"Successfully reset volume levels of all applications", L"Volume Reset", MB_OK);

#endif // _DEBUG


    return 0;
}
