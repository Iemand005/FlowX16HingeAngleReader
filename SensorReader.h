#pragma once

#include <iostream>
#include <string>
#include <iomanip>

#include <Windows.h>

#include <sensorsapi.h>
#include <sensors.h>
#pragma comment(lib, "sensorsapi.lib")


class SensorReader
{
private:
    ISensorManager* pSensorManager = NULL;
    ISensorCollection* pSensorCollection = NULL;
    ISensor* pSensor = NULL;

    void ThrowIfFailed(HRESULT hr) {
        if (FAILED(hr)) throw hr;
    }

public:
    HRESULT Init()
    {
        HRESULT hr = S_OK;


        try {

            ThrowIfFailed(CoInitializeEx(NULL, COINIT_MULTITHREADED));
            ThrowIfFailed(CoCreateInstance(CLSID_SensorManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSensorManager)));

            REFSENSOR_ID sensorID = { 0x020B0000, 0x0000, 0x0000, 0x0000, 0x000000000000 };
            pSensorManager->GetSensorByID(sensorID, &pSensor);
        }
        catch (HRESULT hrError){
            hr = hrError;
        }

        pSensor->Release();
        pSensorCollection->Release();
        pSensorManager->Release();

        return hr;
    }
    
    HRESULT GetHingeSensor(REFSENSOR_CATEGORY_ID sensorCategory, ISensorCollection** ppSensorsFound)
    {
        
        ThrowIfFailed(pSensorManager->GetSensorsByCategory(SENSOR_CATEGORY_ALL, &pSensorCollection));

        ULONG count = 0;
        ThrowIfFailed(pSensorCollection->GetCount(&count));

        if (count == 0)
        {
            cout << "No sensors found." << endl;
            pSensorCollection->Release();
            pSensorManager->Release();
            CoUninitialize();
            return 0;
        }

        int ulIndex = -1;

        while (ulIndex < 0 || ulIndex >= count) {
            cout << "Found " << count << " sensors" << endl;

            for (int i = 0; i < count; i++) {
                if (SUCCEEDED(pSensorCollection->GetAt(i, &pSensor))) {
                    BSTR pSensorName;
                    ThrowIfFailed(pSensor->GetFriendlyName(&pSensorName);
                    cout << "  " << i << ": " << ConvertBSTRToString(pSensorName) << endl;
                    SysFreeString(pSensorName);
                }
            }

            cout << "Choose one: ";
            cin >> ulIndex;
        }

        HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
        COORD dwWriteCoord = { 0, 0 };
        DWORD lpNumberOfItemsWritten = 0;

        CONSOLE_CURSOR_INFO curInfo;
        GetConsoleCursorInfo(hConsoleOutput, &curInfo);
        curInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsoleOutput, &curInfo);

        if (!GetConsoleScreenBufferInfo(hConsoleOutput, &lpConsoleScreenBufferInfo)) return -1;

        DWORD nLength = lpConsoleScreenBufferInfo.dwSize.X * lpConsoleScreenBufferInfo.dwSize.Y;
        FillConsoleOutputCharacterW(hConsoleOutput, L' ', nLength, dwWriteCoord, &lpNumberOfItemsWritten);
        FillConsoleOutputAttribute(hConsoleOutput, lpConsoleScreenBufferInfo.wAttributes, nLength, dwWriteCoord, &lpNumberOfItemsWritten);

        if (SUCCEEDED(pSensorCollection->GetAt(ulIndex, &pSensor))) {
            BSTR pFriendlyName;
            SENSOR_CATEGORY_ID pSensorCategory;
            SENSOR_TYPE_ID pSensorType;

            ThrowIfFailed(pSensor->GetFriendlyName(&pFriendlyName));
            ThrowIfFailed(pSensor->GetCategory(&pSensorCategory));
            ThrowIfFailed(pSensor->GetType(&pSensorType));

            IPortableDeviceKeyCollection* pDataFields = NULL;
            if (SUCCEEDED(pSensor->GetSupportedDataFields(&pDataFields))) {

                DWORD numFields = 0;
                ThrowIfFailed(pDataFields->GetCount(&numFields));

                while (true) {

                    SetConsoleCursorPosition(hConsoleOutput, dwWriteCoord);

                    cout << "  Sensor " << ulIndex << ": " << ConvertBSTRToString(pFriendlyName) << endl;

                    ISensorDataReport* pDataReport = NULL;
                    if (SUCCEEDED(pSensor->GetData(&pDataReport))) {

                        cout << hex << uppercase << setfill('0');

                        for (DWORD j = 0; j < numFields; j++) {
                            PROPERTYKEY pKey;
                            if (SUCCEEDED(pDataFields->GetAt(j, &pKey))) {

                                PROPVARIANT pValue;
                                PropVariantInit(&pValue);

                                if (SUCCEEDED(pDataReport->GetSensorValue(pKey, &pValue))) {

                                    const BYTE* pBytes = reinterpret_cast<const BYTE*>(&pValue) + sizeof(VARTYPE);
                                    constexpr size_t size = sizeof(PROPVARIANT) - sizeof(VARTYPE);

                                    cout << "    ";

                                    for (size_t i = 0; i < size; ++i)
                                        cout << setw(2) << static_cast<int>(pBytes[i]) << " ";

                                    cout << endl;
                                }
                                ThrowIfFailed(PropVariantClear(&pValue));
                            }
                        }
                    }
                    pDataReport->Release();
                }
            }
            pDataFields->Release();
        }
	}
};

