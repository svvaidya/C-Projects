/*
 * Notifier Demo Code
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!! DEMO PURPOSES ONLY !!!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 */



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dbus-1.0/dbus/dbus.h>
#include <math.h>


#define NM_DEVICE_TYPE_ETHERNET  (0x01)
#define NM_DEVICE_TYPE_WIFI      (0x02)
#define NM_DEVICE_TYPE_BT        (0x05)

#define MAX_NUMBER_OF_DEVICES   (10)
#define MAX_DEVICE_NAME_LENGTH  (100)
#define MAX_AP_TO_LIST          (20)
#define MAX_AP_PATH_LENGTH      (100)
#define MAX_AP_SSID_LENGTH      (100)
#define NUM_PROPERTY_TO_READ    (3)

#define NM_STATE_TEST_DURATION  (61)
#define NM_STATE_TEST_DELAY     (5)

#define PRINT_DELAY_TIME        (60)
#define PRINT_TEST_DELAY        (5)

typedef struct
{
    char accessPointPath[MAX_AP_PATH_LENGTH];
    char apSSID[MAX_AP_SSID_LENGTH];
} NM_AP_INFO;

typedef struct
{
    char devicePath[MAX_DEVICE_NAME_LENGTH];
    char ipInterfaceName[50];
    uint32_t interfaceType;
    uint32_t state;
    NM_AP_INFO accessPointList[MAX_AP_TO_LIST];
} NETWORK_DEVICE_INFO;

typedef struct
{
    uint32_t numberOfDevices;
    NETWORK_DEVICE_INFO nmDeviceInfo[MAX_NUMBER_OF_DEVICES];
} NM_DEVICE_LIST;



typedef struct
{
    char infoText[120];
    char printerUri[120];
    char printerName[120];
    char printerStateReason[120];
    uint32_t printerState;
    bool printerIsAcceptingJobs;
} PRINTER_MODIFIED_EVENT_INFO;

bool getPropertyMethodCall(DBusConnection *connection,
                           const char *serviceName,
                           const char *objectPath,
                           const char *interfaceName,
                           const char *propertyName,
                           DBusMessage **reply);

bool sendDbusMethodCall(DBusConnection *busConnection,
                        const char *serviceName,
                        const char *objectPath,
                        const char *interfaceName,
                        const char *methodName,
                        DBusMessage **reply);

bool registerForSignal(DBusConnection *busConnection,
                       const char *interface,
                       const char *path,
                       DBusError *errorResponse);

bool unregisterForSignal(DBusConnection *busConnection,
                         const char *interface,
                         const char *path,
                         DBusError *errorResponse);

const char *dbusServiceName = "org.freedesktop.DBus";
const char *dbusServiceobjectPath = "/org/freedesktop/DBus";
const char *dbusServiceInterface = "org.freedesktop.DBus";
const char *dbusListServiceMethod = "ListNames";

const char *nmServiceName = "org.freedesktop.NetworkManager";
const char *nmServiceobjectPath = "/org/freedesktop/NetworkManager";
const char *nmServiceInterface = "org.freedesktop.NetworkManager"; // NM Service interface
const char *nmDeviceInterface = "org.freedesktop.NetworkManager.Device";



const char *nmGetDeviceListMethod = "GetDevices"; // method name
const char *nmPropertyNameList[NUM_PROPERTY_TO_READ] =  { "Interface", "DeviceType", "State"}; // Ip Interface name of the device

const char *nmDeviceSignalName = "StateChanged";


NM_DEVICE_LIST nmDeviceList;
PRINTER_MODIFIED_EVENT_INFO printerModifiedInfo;

/*
 * getPropertyMethodCall
 */
bool getPropertyMethodCall(DBusConnection *connection,
                           const char *serviceName,
                           const char *objectPath,
                           const char *interfaceName,
                           const char *propertyName,
                           DBusMessage **reply) {
    DBusMessage *sendMsg = NULL;
    DBusMessage *replyMsg = NULL;
    DBusPendingCall *pending;
    bool returnVal = false;

    sendMsg = dbus_message_new_method_call(serviceName,
                                           objectPath,
                                           "org.freedesktop.DBus.Properties", //interfaceName,
                                           "Get");
    if (sendMsg != NULL) {
        // append arguments
        if (dbus_message_append_args(sendMsg,
                                     DBUS_TYPE_STRING, &interfaceName,
                                     DBUS_TYPE_STRING, &propertyName,
                                     DBUS_TYPE_INVALID) == false) {
            printf("%s(%d):: Failed to append arguments to the message\n", __FUNCTION__, __LINE__);

        } else {
            // send message and get a handle for a reply
            if (dbus_connection_send_with_reply (connection,
                                                 sendMsg,
                                                 &pending,
                                                 -1) == true) {  // -1 is default timeout
                if (pending != NULL) {
                    dbus_connection_flush(connection);
                    // block until we receive a response reply
                    dbus_pending_call_block(pending);
                    // get the reply message
                    replyMsg = dbus_pending_call_steal_reply(pending);
                    if (replyMsg != NULL) {
                        returnVal = true;
                    } else {
                        printf("%s(%d):: Failed to send message\n", __FUNCTION__, __LINE__);
                    }
                    // free the message handle pointer
                    dbus_pending_call_unref(pending);
                } else {
                    printf("%s(%d):: Null Pointer for Pending Message Handler\n", __FUNCTION__, __LINE__);
                }
            } else {
                printf("%s(%d):: Unable to allocate memory\n", __FUNCTION__, __LINE__);
            }
        }

        // free message
        dbus_message_unref(sendMsg);
    } else {
        printf("%s(%d):: Failed to send message\n", __FUNCTION__, __LINE__);
    }

    *reply = replyMsg;
    return returnVal;
}

/*
 * sendDbusMethodCall
 */
bool sendDbusMethodCall(DBusConnection *connection,
                        const char *serviceName,
                        const char *objectPath,
                        const char *interfaceName,
                        const char *methodName,
                        DBusMessage **reply)
{
    DBusMessage *sendMsg = NULL;
    DBusMessage *replyMsg = NULL;
    DBusPendingCall *pending;
    bool returnVal = false;

    sendMsg = dbus_message_new_method_call(serviceName,
                                           objectPath,
                                           interfaceName,
                                           methodName);
    if (sendMsg != NULL)
    {
        // send message and get a handle for a reply
        if (dbus_connection_send_with_reply (connection,
                                             sendMsg,
                                             &pending,
                                             -1) == true)   // -1 is default timeout
        {
            if (pending != NULL)
            {
                dbus_connection_flush(connection);
                // block until we recieve a reply
                dbus_pending_call_block(pending);
                // get the reply message
                replyMsg = dbus_pending_call_steal_reply(pending);
                if (replyMsg != NULL)
                {
                    returnVal = true;
                }
                else
                {
                    printf("%s(%d)::NULL Pointer for Reply message\n", __FUNCTION__, __LINE__);
                }
                // free the pending message handle
                dbus_pending_call_unref(pending);
            }
            else
            {
                printf("%s(%d):: Null Pointer for Pending Message Handler\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            printf("%s(%d):: Unable to allocate memory\n", __FUNCTION__, __LINE__);
        }
        // free message
        dbus_message_unref(sendMsg);
    }
    else
    {
        printf("%s(%d):: Failed to send message\n", __FUNCTION__, __LINE__);
    }

    *reply = replyMsg;
    return returnVal;
}

/*
 * registerForSignal
 */

bool registerForSignal(DBusConnection *busConnection,
                       const char *interface,
                       const char *path,
                       DBusError *errorResponse)
{
    char signalConfiguration[512];
    bool status = false;

    if (path == NULL)
    {
        snprintf(signalConfiguration, sizeof(signalConfiguration), "type='signal',interface='%s'", interface);
    }
    else
    {
        snprintf(signalConfiguration, sizeof(signalConfiguration), "type='signal',path='%s',interface='%s'", path, interface);
    }

    dbus_bus_add_match(busConnection, (const char *)signalConfiguration, errorResponse);
    if (dbus_error_is_set(errorResponse) == false)
    {
        dbus_connection_flush(busConnection);
        if (dbus_error_is_set(errorResponse) == false)
        {
            status = true;
        }
    }

    return status;
}

bool unregisterForSignal(DBusConnection *busConnection,
                         const char *interface,
                         const char *path,
                         DBusError *errorResponse)
{
    char signalConfiguration[512];
    bool status = false;

    if (path == NULL)
    {
        snprintf(signalConfiguration, sizeof(signalConfiguration), "type='signal',interface='%s'", interface);
    }
    else
    {
        snprintf(signalConfiguration, sizeof(signalConfiguration), "type='signal',path='%s',interface='%s'", path, interface);
    }

    dbus_bus_remove_match(busConnection, (const char *)signalConfiguration, errorResponse);
    if (dbus_error_is_set(errorResponse) == false)
    {
        dbus_connection_flush(busConnection);
        if (dbus_error_is_set(errorResponse) == false)
        {
            status = true;
        }
    }

    return status;
}

/*
 * Main Function
 */

static const uint32_t TEST_MODE_NET = 0x01;
static const uint32_t TEST_MODE_PRINT = 0x02;

int main (int argc, char *argv[])
{
    DBusMessage *replyMsg;
    DBusConnection *busConnection;
    DBusError errorResponse;
    DBusMessageIter responseArguments;
    DBusMessageIter responseArgumentArray;
    bool status = false;
    char *objectPath;
    int32_t argumentType;
    uint32_t uint32Response;
    uint32_t delayCounter = 0;
    uint32_t deviceCounter = 0;
    uint32_t deviceType;
    uint32_t argumentCounter = 0;
    uint32_t signalCounter = 0;
    uint32_t propertyCounter;
    uint32_t test_mode = 0;
    uint32_t test_time_net = NM_STATE_TEST_DURATION;
    uint32_t test_time_print = PRINT_DELAY_TIME;
    int argCount;
    int arrayLen;

    // Parse the command line arguments
    if (1 < argc)
    {
        // Check if network was selected
        if (!strcmp("network", argv[1]) || !strcmp("net", argv[1]))
        {
            test_mode |= TEST_MODE_NET;

            if (2 < argc)    // Third argument is the network test length
            {
                test_time_net = atoi(argv[2]);
            }

            // printf("Net Only! - %d\n", test_mode);
        }
        // Check if print was selected
        else if (!strcmp("print", argv[1]))
        {
            test_mode |= TEST_MODE_PRINT;

            if (2 < argc)    // Third argument is the print test length
            {
                test_time_print = atoi(argv[2]);
            }

        	// printf("Print Only! - %d  Time - %d\n", test_mode, test_time_print);
        }

        else if (!strcmp("all", argv[1]))
        {
            test_mode |= TEST_MODE_NET;
            test_mode |= TEST_MODE_PRINT;

            if (2 < argc)    // Third argument is the network test length
            {
                test_time_net = atoi(argv[2]);
            }
            if (3 < argc)    // Fourth argument is the print test length
            {
                test_time_print = atoi(argv[3]);
            }

            // printf("All - %d\n", test_mode);
        }
        // else - Run both tests and use default time
        else
        {
            test_mode |= TEST_MODE_NET;
            test_mode |= TEST_MODE_PRINT;

            // printf("Default - %d\n", test_mode);
        }
    }
    else
    {
        test_mode |= TEST_MODE_NET;
        test_mode |= TEST_MODE_PRINT;

        // printf("Default - %d\n", test_mode);
    }


    // initialize the errors
    dbus_error_init(&errorResponse);

    // connect to the system bus and check for errors
    busConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &errorResponse);
    if (dbus_error_is_set(&errorResponse))
    {
        printf("Error:: Connecting to bus:: (%s)\n", errorResponse.message);
        dbus_error_free(&errorResponse);
    }

    if (busConnection == NULL)
    {
        exit(1);
    }


    /***********************************************************************************************
     *
     * Network Notifications
     *
     ***********************************************************************************************/

    if (TEST_MODE_NET & test_mode)
    {
        // create a new method call and check for errors
        status = sendDbusMethodCall(busConnection,
                                    nmServiceName,
                                    nmServiceobjectPath,
                                    nmServiceInterface,
                                    nmGetDeviceListMethod,
                                    &replyMsg);

        if ((status == true) && (replyMsg != NULL))
        {
            if (!dbus_message_iter_init(replyMsg, &responseArguments))
            {
                printf("Reply Message has no arguments!\n");
            }
            else
            {
                //printf( "Argument type is %d\n", dbus_message_iter_get_arg_type(&responseArguments)) ;
                nmDeviceList.numberOfDevices =  dbus_message_iter_get_element_count(&responseArguments);
                if (nmDeviceList.numberOfDevices > MAX_NUMBER_OF_DEVICES)
                {
                    nmDeviceList.numberOfDevices = MAX_NUMBER_OF_DEVICES;
                }

                deviceCounter = 0;
                do
                {
                    //printf("\n\n\nCounter = %d .......\n", (loopCounter + 1));
                    argumentType = dbus_message_iter_get_arg_type (&responseArguments);
                    //printf("%u:: type = %u\n", deviceCounter, argumentType);
                    dbus_message_iter_recurse(&responseArguments, &responseArgumentArray);
                    do
                    {
                        dbus_message_iter_get_basic(&responseArgumentArray, &objectPath);
                        snprintf(nmDeviceList.nmDeviceInfo[deviceCounter].devicePath,
                                 MAX_DEVICE_NAME_LENGTH,
                                 "%s",
                                 objectPath);
                        deviceCounter++;
                    }
                    while (dbus_message_iter_next(&responseArgumentArray));
                }
                while (dbus_message_iter_next(&responseArguments));

                printf("Detected Device List:\n");
                for (deviceCounter = 0; deviceCounter < nmDeviceList.numberOfDevices; deviceCounter++)
                {
                    printf("%u:: %s\n", deviceCounter, nmDeviceList.nmDeviceInfo[deviceCounter].devicePath);
                }
                printf("\n\n");
            }

            // free reply and close connection
            dbus_message_unref(replyMsg);
        }

        for (deviceCounter = 0; deviceCounter < nmDeviceList.numberOfDevices; deviceCounter++)
        {
            for (propertyCounter = 0; propertyCounter < NUM_PROPERTY_TO_READ; propertyCounter++)
            {
                status = getPropertyMethodCall(busConnection,
                                               nmServiceName,
                                               (const char *)nmDeviceList.nmDeviceInfo[deviceCounter].devicePath,
                                               nmDeviceInterface,
                                               nmPropertyNameList[propertyCounter],
                                               &replyMsg);
                if ((status == true) && (replyMsg != NULL))
                {
                    if (!dbus_message_iter_init(replyMsg, &responseArguments))
                    {
                        printf("Reply Message has no arguments!\n");
                    }
                    else
                    {
                        //printf( "%d:: Argument type is %d\n", deviceCounter, dbus_message_iter_get_arg_type(&responseArguments));
                        //printf("%d:: Element count = %d\n", deviceCounter, dbus_message_iter_get_element_count(&responseArguments));
                        if (dbus_message_iter_get_arg_type(&responseArguments) != DBUS_TYPE_VARIANT)
                        {
                            dbus_set_error_const(&errorResponse, "Reply type error", "Response type is not a 'variant'");
                        }
                        else
                        {
                            dbus_message_iter_recurse(&responseArguments, &responseArgumentArray);
                            argumentType =  dbus_message_iter_get_arg_type(&responseArgumentArray);
                            //printf( "%d:: Argument type is %d\n", deviceCounter, argumentType);

                            if (argumentType == DBUS_TYPE_STRING)
                            {
                                dbus_message_iter_get_basic(&responseArgumentArray, &objectPath);
                                if(propertyCounter == 0)
                                {
                                    snprintf(nmDeviceList.nmDeviceInfo[deviceCounter].ipInterfaceName,
                                             sizeof(nmDeviceList.nmDeviceInfo[deviceCounter].ipInterfaceName),
                                             "%s",  objectPath);
                                    //printf("%d:: %s = %s\n", deviceCounter, "IpInterface", nmDeviceList.nmDeviceInfo[deviceCounter].ipInterfaceName);
                                }
                                else
                                {
                                    printf("Error: Property %s does not expect String Data (%s\n)",
                                           nmPropertyNameList[propertyCounter], objectPath);
                                }
                            }
                            else if (argumentType == DBUS_TYPE_UINT32)
                            {
                                dbus_message_iter_get_basic(&responseArgumentArray,&uint32Response);

                                if (propertyCounter == 1)
                                {
                                    nmDeviceList.nmDeviceInfo[deviceCounter].interfaceType = uint32Response;
                                }
                                else
                                {
                                    nmDeviceList.nmDeviceInfo[deviceCounter].state = uint32Response;
                                }
                            }
                            else
                            {
                                dbus_set_error_const(&errorResponse, "Argument Type error: ", "Expected 'string' or 'uint32' argument");
                            }
                        }
                    }
                }
            }
        }

        for (deviceCounter = 0; deviceCounter < nmDeviceList.numberOfDevices; deviceCounter++)
        {
            printf("Device Number %u\n", deviceCounter);
            printf("    Device Name: %s\n", nmDeviceList.nmDeviceInfo[deviceCounter].devicePath);
            printf("    Device Interface Name: %s\n", nmDeviceList.nmDeviceInfo[deviceCounter].ipInterfaceName);
            printf("    Device Type: %u :: ", nmDeviceList.nmDeviceInfo[deviceCounter].interfaceType);
            if (nmDeviceList.nmDeviceInfo[deviceCounter].interfaceType == NM_DEVICE_TYPE_ETHERNET)
            {
                printf("Ethernet device\n");
            }
            else if (nmDeviceList.nmDeviceInfo[deviceCounter].interfaceType == NM_DEVICE_TYPE_WIFI)
            {
                printf("WIFI device\n");
            }
            else
            {
                printf("other device\n");
            }
            printf("    Device State: %u\n\n", nmDeviceList.nmDeviceInfo[deviceCounter].state);
        }

        for (deviceCounter = 0; deviceCounter < nmDeviceList.numberOfDevices; deviceCounter++)
        {
            deviceType = nmDeviceList.nmDeviceInfo[deviceCounter].interfaceType;

            printf("TYPE %d: Interface %s PATH %s\n", deviceType, 
                                                      nmDeviceInterface, 
                                                      nmDeviceList.
                                                      nmDeviceInfo[deviceCounter].devicePath);

            if ((deviceType == NM_DEVICE_TYPE_ETHERNET) || (deviceType == NM_DEVICE_TYPE_WIFI))
            {
                printf("Starting %d second 'StateChanged' Signal Test:\n", test_time_net);
                printf("    Device Name: %s  Type:: %s\n", nmDeviceList.nmDeviceInfo[0].devicePath,
                       (deviceType == NM_DEVICE_TYPE_WIFI) ? "WIFI" : "Ethernet");

                status = registerForSignal(busConnection,
                                           nmDeviceInterface,
                                           nmDeviceList.nmDeviceInfo[deviceCounter].devicePath,
                                           &errorResponse);
                if (status == true)
                {
                    delayCounter = 0;
                    signalCounter = 0;
                    // loop listening for signals being emitted
                    while (delayCounter < test_time_net)
                    {
                        delayCounter++;
                        // non blocking read of the next available message
                        dbus_connection_read_write(busConnection, 0);
                        replyMsg = dbus_connection_pop_message(busConnection);

                        // loop again if we haven't read a message
                        if (replyMsg == NULL)
                        {
                            sleep(1);
                            continue;
                        }

                        // check if the message is a signal from the correct interface and with the correct name
                        if (dbus_message_is_signal(replyMsg,
                                                   nmDeviceInterface,
                                                   nmDeviceSignalName))
                        {
                            signalCounter++;
                            printf("%d:: State Change Signal received on Device %s\n",
                                   signalCounter,
                                   nmDeviceList.nmDeviceInfo[deviceCounter].devicePath);
                            // read the parameters
                            if (!dbus_message_iter_init(replyMsg, &responseArguments))
                            {
                                printf("Message has no arguments!\n");
                            }
                            else
                            {
                                // argCount = dbus_message_iter_get_element_count(&responseArguments);
                                // arrayLen = dbus_message_iter_get_array_len(&responseArguments);
                                // printf("Array Element Type = %d \n", argCount);

                                argumentCounter = 0;
                                do
                                {
                                    argumentType = dbus_message_iter_get_arg_type (&responseArguments);
                                    printf("Argument counter %d Argument type = %d\n", argumentCounter, argumentType);
                                    dbus_message_iter_get_basic(&responseArguments, &uint32Response);
                                    if (argumentCounter == 0)
                                    {
                                        printf("    New State: %u\n", uint32Response);
                                    }
                                    else if (argumentCounter == 1)
                                    {
                                        printf("    Old State: %u\n", uint32Response);
                                    }
                                    else if (argumentCounter == 2)
                                    {
                                        printf("    Reason Code: %u\n", uint32Response);
                                    }
                                    else
                                    {
                                        printf("Unknown argument: %u\n", uint32Response);
                                    }

                                    argumentCounter++;
                                }
                                while (dbus_message_iter_next(&responseArguments));

                                // Determine the interface that has generated this message
                                printf("\n");
                            }
                        }

                        // free the message
                        dbus_message_unref(replyMsg);
                    }

                    printf("De-registering signal 'StateChanged' for device:: %s\n", nmDeviceList.nmDeviceInfo[deviceCounter].devicePath);
                    status = unregisterForSignal(busConnection,
                                                 nmDeviceInterface,
                                                 nmDeviceList.nmDeviceInfo[deviceCounter].devicePath,
                                                 &errorResponse);
                    if (status == true)
                    {
                        printf("Signal de-registered successfully\n");
                        printf("Starting %d second Delay loop\n", NM_STATE_TEST_DELAY);
                        delayCounter = 0;
                        signalCounter = 0;
                        while (delayCounter < NM_STATE_TEST_DELAY)
                        {
                            delayCounter++;
                            // non blocking read of the next available message
                            dbus_connection_read_write(busConnection, 0);
                            replyMsg = dbus_connection_pop_message(busConnection);

                            // loop again if we haven't read a message
                            if (replyMsg == NULL)
                            {
                                sleep(1);
                                continue;
                            }

                            // check if the message is a signal from the correct interface and with the correct name
                            if (dbus_message_is_signal(replyMsg,
                                                       nmDeviceInterface,
                                                       nmDeviceSignalName))
                            {
                                printf("!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                                printf("!!! Signal received on un-registered interface !!!!!\n" );
                                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                            }
                        }
                    }
                    else
                    {
                        printf("Failed to de-register signal\n");
                    }

                }
                else
                {
                    printf("Error:: Failed to register for signal:: (%s)\n", errorResponse.message);
                    dbus_error_free(&errorResponse);
                }
            }
        }
    }
    else
    {
        sleep(5);
    }

    /***********************************************************************************************
     *
     * Printer Notifications
     *
     ***********************************************************************************************/

    if (TEST_MODE_PRINT & test_mode)
    {
        printf("\n\n!!!!!! Starting Printer test in %d seconds !!!!!!!!\n\n", PRINT_TEST_DELAY);
        delayCounter = 0;
        while (delayCounter < PRINT_TEST_DELAY)
        {
            sleep(1);
            delayCounter++;
        }

        printf("Starting %d second 'Printer' Signal Test:\n", test_time_print);

        status = registerForSignal(busConnection, "org.cups.cupsd.Notifier", NULL, &errorResponse);
        printf("Print 1\n");
        if (status == true)
        {
            delayCounter = 0;
            signalCounter = 0;

            // loop listening for signals being emitted
            while (delayCounter < test_time_print)
            {
                delayCounter++;
                // non blocking read of the next available message
                dbus_connection_read_write(busConnection, 0);
                replyMsg = dbus_connection_pop_message(busConnection);

                // loop again if we haven't read a message
                if (replyMsg == NULL)
                {
                    sleep(1);
                    continue;
                }

                // check if the message is a signal from the correct interface and with the correct name
                const char* signature = dbus_message_get_signature(replyMsg);
                char* signame = NULL;

                //           if (dbus_message_is_signal(replyMsg,
                //                                    "org.cups.cupsd.Notifier",
                //                                   "PrinterModified")) {
                //               signalCounter++;
                //               printf("%d:: PrinterMethod Signal received\n", signalCounter);
                //           }

                signalCounter++;
                printf("%d:: %s Signal signature\n", signalCounter, signature);

                // read the parameters
                if (!dbus_message_iter_init(replyMsg, &responseArguments))
                {
                    printf("Message has no arguments!\n");
                }
                else
                {
                    // printf("Parameter read\n");
                    argumentCounter = 0;
                    do
                    {
                        argumentType = dbus_message_iter_get_arg_type (&responseArguments);
                        // printf("%s(%d) %d:: type = %c\n", __FUNCTION__, __LINE__, argumentCounter, argumentType);

                        if (argumentType == DBUS_TYPE_STRING)
                        {
                            // printf("String: %s\n", objectPath);

                            dbus_message_iter_get_basic(&responseArguments, &objectPath);
                            switch (argumentCounter)
                            {
                                case 0:
                                    snprintf(printerModifiedInfo.infoText, sizeof(printerModifiedInfo.infoText), "%s", objectPath);
                                    break;

                                case 1:
                                    snprintf(printerModifiedInfo.printerUri, sizeof(printerModifiedInfo.printerUri), "%s", objectPath);
                                    break;

                                case 2:
                                    snprintf(printerModifiedInfo.printerName, sizeof(printerModifiedInfo.printerName), "%s", objectPath);
                                    break;

                                case 4:
                                    snprintf(printerModifiedInfo.printerStateReason, sizeof(printerModifiedInfo.printerStateReason), "%s", objectPath);
                                    break;

                                default:
                                    break;
                            }

                        }
                        else if (argumentType == DBUS_TYPE_UINT32)
                        {
                            // printf("Print State %d\n", printerModifiedInfo.printerState);
                            dbus_message_iter_get_basic(&responseArguments,&printerModifiedInfo.printerState );
                        }
                        else if (argumentType == DBUS_TYPE_BOOLEAN)
                        {
                            // printf("Print State %d\n", printerModifiedInfo.printerIsAcceptingJobs);
                            dbus_message_iter_get_basic(&responseArguments,&printerModifiedInfo.printerIsAcceptingJobs);
                        }
                        else
                        {
                            dbus_set_error_const(&errorResponse, "Argument Type error: ", "Expected 'string' or 'uint32' argument");
                        }

                        // dbus_message_iter_get_basic(&responseArguments, &uint32Response);
                        // printf("Arg %d:: Value = %u 0x%08x\n", (deviceCounter + 1), uint32Response, uint32Response);
                        argumentCounter++;
                    }
                    while (dbus_message_iter_next(&responseArguments));

                    printf("%s Event details:\n", signame);
                    printf("    Info: %s\n", printerModifiedInfo.infoText);
                    printf("    Printer URI: %s\n", printerModifiedInfo.printerUri);
                    printf("    Printer Name: %s\n", printerModifiedInfo.printerName);
                    printf("    Printer State: %u\n", printerModifiedInfo.printerState);
                    printf("    Printer State Reason: %s\n", printerModifiedInfo.printerStateReason);
                    printf("    Printer Accepting Jobs: %s\n", (printerModifiedInfo.printerIsAcceptingJobs == true) ? "Yes" : "No");
                    printf("\n");
                }

                // free the message
                dbus_message_unref(replyMsg);
            }

            printf("De-registering signal 'PrinterModified'\n");
            status = unregisterForSignal(busConnection, "org.cups.cupsd.Notifier", NULL, &errorResponse);
            if (status == true)
            {
                printf("Printer de-registered successfully\n");
                printf("Starting %d second Delay loop\n", PRINT_TEST_DELAY);
                delayCounter = 0;
                signalCounter = 0;
                while (delayCounter < PRINT_TEST_DELAY)
                {
                    delayCounter++;
                    // non blocking read of the next available message
                    dbus_connection_read_write(busConnection, 0);
                    replyMsg = dbus_connection_pop_message(busConnection);

                    // loop again if we haven't read a message
                    if (replyMsg == NULL)
                    {
                        sleep(1);
                        continue;
                    }

                    // check if the message is a signal from the correct interface and with the correct name
                    if (dbus_message_is_signal(replyMsg,
                                               "org.cups.cupsd.Notifier",
                                               "PrinterModified"))
                    {
                        printf("!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                        printf("!!! Signal received on un-registered interface !!!!!\n" );
                        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    }
                }
            }
            else
            {
                printf("Failed to de-register signal\n");
            }

        }
        else
        {
            printf("Error:: Failed to register for signal:: (%s)\n", errorResponse.message);
            dbus_error_free(&errorResponse);
        }
    }

    return 0;
}

/*
 * Installing dbus  libraries
 * sudo apt-get -y install dbus libdbus-1-dev libdbus-glib-1-2 libdbus-glib-1-dev
 *
 * Compilation Command:
gcc main.c -I/usr/include/dbus-1.0 \
           -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include \
           -I/usr/include/glib-2.0 \
           -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ \
           -ldbus-1 -ldbus-glib-1 \
           -Wall -Wextra -o dbus-demo

*/

/*
 * Regular:
 * gcc main.c -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include   -ldbus-1 -Wall -Wextra -o dbus-demo
 *
 * Debug:
 * gcc main.c -g -O0 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include   -ldbus-1 -Wall -Wextra -o dbus-demo
 */
