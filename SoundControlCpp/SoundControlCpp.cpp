//Pobiera identyfikator GUID do³¹czony do wyra¿enia. = _uuidof

#include <iostream>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <combaseapi.h>
#include <audiopolicy.h>
#include <Unknwn.h>
#include <Psapi.h>

#define NULL nullptr
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

using namespace std;

string get_process_name(DWORD processID) {
	const string NULL_MESSAGE = "NULL";
	HANDLE Handle = OpenProcess(
		PROCESS_ALL_ACCESS,
		FALSE,
		processID
	);
	if (Handle)
	{
		char Buffer[MAX_PATH];
		if (GetProcessImageFileNameA(Handle, Buffer, MAX_PATH))
		{
			CloseHandle(Handle);
			return  std::string(Buffer);
		}

		CloseHandle(Handle);
		return NULL_MESSAGE;
	}
	else {
		return NULL_MESSAGE;
	}
}

string purify_name(string name) {
	int index = name.find_last_of("\\");
	name = name.substr(index+1);
	return name;
}

int main() {
	string list_to_mute[2] = { "Spotify.exe", "opera.exe" };

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
	DWORD process_id;

	cout << "Error Code (Before Loop): " << hr << endl;
	for (int i = 0; i < session_count; i++) {
		hr = session_enum->GetSession(i, &session_control);
		hr = session_control->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audio_volume);
		hr = session_control->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&session_control2);
		hr = session_control2->GetProcessId(&process_id);
		hr = session_control2->GetDisplayName(&display_name);
		//hr = audio_volume->SetMute(true, 0);
		string process_name = "";
		try {
			process_name = get_process_name(process_id);
			process_name = purify_name(process_name);
		}
		catch(exception){

		}
		cout << "Session [" << i + 1 << "]: " <<  process_name << "->" << process_id << endl;

		if (std::find(std::begin(list_to_mute), std::end(list_to_mute), process_name) != std::end(list_to_mute)) {
			audio_volume->SetMute(true, 0);
		}

		SAFE_RELEASE(session_control);
		display_name = 0;
		SAFE_RELEASE(audio_volume);
		SAFE_RELEASE(session_control2);
		process_id = 0;
	}

	cout << "Error Code: " << hr << endl;
	cout << "Volume: " << current_vol << endl;
	
	return 0;

Exit:
	printf("Error!\n");
}