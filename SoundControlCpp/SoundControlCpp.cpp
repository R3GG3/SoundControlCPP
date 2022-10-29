#include <iostream>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <combaseapi.h>
#include <audiopolicy.h>
#include <Psapi.h>
#include <fstream>
#include <string>
#include <Windows.h>
#include<thread>
#include <list>


#define NULL nullptr
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const SHORT MUTE_KEYBIND = 0x61;
const SHORT UNMUTE_KEYBIND = 0x60;
const SHORT EXIT_KEYBIND = 0x63;

using namespace std;

class SoundControl {
private:
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

	int session_count = 0;
	DWORD process_id;
	std::list<string> list_to_mute = {};


	string purify_name(string name) {
		int index = name.find_last_of("\\");
		name = name.substr(index + 1);
		return name;
	}

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

public:
	SoundControl(string filename) {
		LoadList(filename);

		hr = CoInitialize(0);
		hr = CoCreateInstance(
			CLSID_MMDeviceEnumerator, NULL,
			CLSCTX_ALL, IID_IMMDeviceEnumerator,
			(void**)&audio_device_enum);


		hr = audio_device_enum->GetDefaultAudioEndpoint(eRender, eConsole, &audio_device);

		hr = audio_device->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&session_manager);

		LPCGUID quered_interface;
		hr = session_manager->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&quered_interface);
		hr = session_manager->GetSimpleAudioVolume(quered_interface, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, &audio_volume);
		hr = session_manager->GetAudioSessionControl(quered_interface, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, &session_control);
		hr = session_manager->QueryInterface(__uuidof(IAudioSessionManager2), (void**)&session_manager2);
		hr = session_control->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&session_control2);
		hr = session_manager2->GetSessionEnumerator(&session_enum);
		
		SAFE_RELEASE(session_manager);
		SAFE_RELEASE(session_manager2);
		SAFE_RELEASE(audio_device);
		SAFE_RELEASE(audio_device_enum);
		delete quered_interface;
	}

	~SoundControl() {
		SAFE_RELEASE(session_enum);
		SAFE_RELEASE(session_control);
		SAFE_RELEASE(session_control2);
	}

	void Get_process_id(int i) {
		hr = session_enum->GetSession(i, &session_control);
		hr = session_control->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audio_volume);
		hr = session_control->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&session_control2);
		hr = session_control2->GetProcessId(&process_id);
	}

	void Mute() {
		hr = session_enum->GetCount(&session_count);
		for (int i = 0; i < session_count; i++) {
			Get_process_id(i);
			string process_name = "";
			try {
				process_name = get_process_name(process_id);
				process_name = purify_name(process_name);
			}
			catch (exception) {

			}
			//if (process_id != 0 && process_name != "SongMuter.exe") {
				//cout << "Session [" << i + 1 << "]: " << process_name << "->" << process_id << endl;
			//}
			if (std::find(list_to_mute.begin(), list_to_mute.end(), process_name) != list_to_mute.end()) {
				audio_volume->SetMute(true, 0);
			}
		}
	}

	void UnMute() {
		hr = session_enum->GetCount(&session_count);
		for (int i = 0; i < session_count; i++) {
			Get_process_id(i);
			string process_name = "";
			try {
				process_name = get_process_name(process_id);
				process_name = purify_name(process_name);
			}
			catch (exception) {

			}
			//if (process_id != 0 && process_name != "SoundControlCpp.exe") {
				//cout << "Session [" << i + 1 << "]: " << process_name << "->" << process_id << endl;
			//}

			if (std::find(list_to_mute.begin(), list_to_mute.end(), process_name) != list_to_mute.end()) {
				{
					audio_volume->SetMute(false, 0);
				}
			}
		}
	}

	void LoadList(string filename)
	{
		ifstream MyReadFile(filename);
		string myText;

		while (std::getline(MyReadFile, myText))
		{
			list_to_mute.insert(list_to_mute.end(), myText);
		}
	}
};

void ListenKey() {
	SoundControl sound_control("list.txt");
	while (true) {
		if (GetKeyState(MUTE_KEYBIND) & 0x8000) {
			sound_control.Mute();
			//cout << "[Muted]" << endl;
			Sleep(1000);
		}

		else if (GetKeyState(UNMUTE_KEYBIND) & 0x8000) {
			sound_control.UnMute();
			//cout << "[Unmuted]" << endl;
			Sleep(1000);
		}

		else if (GetKeyState(EXIT_KEYBIND) & 0x8000) {
			break;
		}
	}
}

int main() {
	HWND window;
	AllocConsole();
	window = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(window, 0);

	ListenKey();
	return 0;
}