//========= Copyright OpenMod, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "workshopmanager.h"
#include "vgui_controls/Frame.h"

using namespace vgui;

char* getList(const char* link)
{
    HINTERNET hInternet = InternetOpen("HTTP GET Request", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL)
    {
        return nullptr;
    }

    HINTERNET hConnect = InternetOpenUrl(hInternet, link, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect == NULL)
    {
        InternetCloseHandle(hInternet);
        return nullptr;
    }

    DWORD bufferSize = 4096;
    char* response = new char[bufferSize];
    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;

    while (InternetReadFile(hConnect, response + totalBytesRead, bufferSize - totalBytesRead, &bytesRead) && bytesRead > 0)
    {
        totalBytesRead += bytesRead;

        if (totalBytesRead == bufferSize)
        {
            bufferSize *= 2;
            char* newResponse = new char[bufferSize];
            memcpy(newResponse, response, totalBytesRead);
            delete[] response;
            response = newResponse;
        }
    }

    response[totalBytesRead] = '\0';

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
}

void CmdGetList(const CCommand& args)
{
    const char* link = (args.ArgC() > 1) ? args.Arg(1) : "https://jsonplaceholder.typicode.com/todos/1";
    const char* result = getList(link);

    if (result != nullptr)
    {
        struct json_value_s* root = json_parse(result, strlen(result));
        assert(root != NULL);

        struct json_object_s* object = json_value_as_object(root);
        assert(object != NULL);

        struct json_object_element_s* userId_elem = object->start;
        while (userId_elem != NULL) {
            struct json_string_s* name = userId_elem->name;
            if (strcmp(name->string, "userId") == 0) {
                struct json_value_s* userId_value = userId_elem->value;
                assert(userId_value != NULL);

                if (userId_value->type == json_type_number) {
                    Msg("%s\n", *(int*)userId_value->payload);
                }

                break;
            }
            userId_elem = userId_elem->next;
        }

        free(root);

        delete[] result;
    }
}

static ConCommand getlistCommand("getlist", CmdGetList, "Fetch data from a URL and print it to the console", FCVAR_NONE);