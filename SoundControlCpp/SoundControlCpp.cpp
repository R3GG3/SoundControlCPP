//Pobiera identyfikator GUID do³¹czony do wyra¿enia. = _uuidof

#include <iostream>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <combaseapi.h>
#include <audiopolicy.h>
#include <Unknwn.h>

#define NULL nullptr
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

using namespace std;

int main() {
	IAudioSessionControl* session_control = NULL;
	IAudioSessionControl2* session_control2 = NULL;
	IAudioSessionManager* session_manager = NULL;
	IAudioSessionManager2* session_manager2 = NULL;
	IAudioSessionEnumerator* session_enum = NULL;

	IMMDevice* audio_device = NULL;
	IMMDeviceEnumerator* audio_device_enum = NULL;
	HRESULT hr = S_OK;

	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const CLSID CLSID_IAudioSessionEnumerator = __uuidof(IAudioSessionEnumerator);
	const IID IID_IAudioSessionEnumerator = __uuidof(IAudioSessionEnumerator);

	ISimpleAudioVolume* audio_volume = NULL;
	const float chosen_vol = 0;
	float current_vol = 0;

	hr = CoInitialize(0);
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&audio_device_enum);


	hr = audio_device_enum->GetDefaultAudioEndpoint(eRender, eConsole, &audio_device);

	DWORD device_state;
	hr = audio_device->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&session_manager);
	hr = audio_device->GetState(&device_state);
	cout << "Device State: " << device_state << endl;

	LPCGUID quered_interface;
	hr = session_manager->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&quered_interface);
	hr = session_manager->GetSimpleAudioVolume(quered_interface, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, &audio_volume);
	hr = session_manager->GetAudioSessionControl(quered_interface, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, &session_control);
	hr = session_manager->QueryInterface(__uuidof(IAudioSessionManager2), (void**)&session_manager2);
	hr = session_control->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&session_control2);
	hr = audio_volume->GetMasterVolume(&current_vol);
	hr = session_manager2->GetSessionEnumerator(&session_enum);

	int session_count = 0;
	hr = session_enum->GetCount(&session_count);
	
	
	cout << "Session Count: " << session_count << endl;

	LPWSTR display_name;
	for (int i = 0; i < session_count; i++) {
		hr = session_enum->GetSession(i, &session_control);
		hr = session_control->GetDisplayName(&display_name);
		hr = session_control->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audio_volume);
		hr = audio_volume->SetMute(true, 0);

		cout << "Session [" << i + 1 << "]: " << display_name << endl;
	}

	cout << "Error Code: " << hr << endl;
	cout << "Volume: " << current_vol << endl;
	
	return 0;

Exit:
	printf("Error!\n");
}