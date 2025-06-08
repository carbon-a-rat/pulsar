// PocketbaseArduino.cpp

#include "pocketbase.hpp"
// #include <ESP8266HTTPClient.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <BearSSLHelpers.h>

#include <json_parser.h>

#include <ArduinoJson.h>

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

PocketbaseArduino::PocketbaseArduino(const char *baseUrl)
{
    base_url = baseUrl;

    if (base_url.endsWith("/"))
    {
        base_url.remove(base_url.length() - 1);
    }

    base_url += "/api/";

    current_endpoint = base_url;
    expand_param = "";
    fields_param = "";

    // Initialize the main connection
    main_connection.base_url = base_url;
    main_connection.auth_token = "";
    // Initialize the client

    main_connection.client = new WiFiClientSecure;
    main_connection.client->setInsecure();

    for (size_t i = 0; i < sizeof(subscription_ctx) / sizeof(subscription_ctx[0]); i++)
    {
        subscription_ctx[i].active = false;
        subscription_ctx[i].callback = nullptr;
        subscription_ctx[i].ctx = nullptr;
        subscription_ctx[i].tcp_connection = nullptr;
        subscription_ctx[i].endpoint = "";
        subscription_ctx[i].collection = "";
        subscription_ctx[i].recordid = "";
    }
}

PocketbaseArduino &PocketbaseArduino::collection(const char *collection)
{
    current_endpoint = "collections/" + String(collection) + "/";
    return *this;
}

String PocketbaseArduino::getOne(const char *recordId, const char *expand /* = nullptr */, const char *fields /* = nullptr */)
{
    String fullEndpoint = base_url + String(current_endpoint) + "records/" + recordId;

    // Append the expand parameter if provided
    if (expand != nullptr && strlen(expand) > 0)
    {
        fullEndpoint += "?expand=" + String(expand);
    }

    // Append the fields parameter if provided
    if (fields != nullptr && strlen(fields) > 0)
    {
        // Check if there's already a query string
        fullEndpoint += (fullEndpoint.indexOf('?') == -1) ? "?" : "&";
        fullEndpoint += "fields=" + String(fields);
    }

    return main_connection.performGETRequest(fullEndpoint.c_str());
}

String PocketbaseArduino::getList(
    const char *page /* = nullptr */,
    const char *perPage /* = nullptr */,
    const char *sort /* = nullptr */,
    const char *filter /* = nullptr */,
    const char *skipTotal /* = nullptr */,
    const char *expand /* = nullptr */,
    const char *fields /* = nullptr */)
{
    String fullEndpoint = base_url + String(current_endpoint) + "records";

    // Append the expand parameter if provided
    if (expand != nullptr && strlen(expand) > 0)
    {
        fullEndpoint += (fullEndpoint.indexOf('?') == -1) ? "?" : "&";
        fullEndpoint += "expand=" + String(expand);
    }

    // Append the fields parameter if provided
    if (fields != nullptr && strlen(fields) > 0)
    {
        // Check if there's already a query string
        fullEndpoint += (fullEndpoint.indexOf('?') == -1) ? "?" : "&";
        fullEndpoint += "fields=" + String(fields);
    }

    // Append the page parameter if provided
    if (page != nullptr && strlen(page) > 0)
    {
        // Check if there's already a query string
        fullEndpoint += (fullEndpoint.indexOf('?') == -1) ? "?" : "&";
        fullEndpoint += "page=" + String(page);
    }

    // Append the perPage parameter if provided
    if (perPage != nullptr && strlen(perPage) > 0)
    {
        // Check if there's already a query string
        fullEndpoint += (fullEndpoint.indexOf('?') == -1) ? "?" : "&";
        fullEndpoint += "perPage=" + String(perPage);
    }

    // Append the sort parameter if provided
    if (sort != nullptr && strlen(sort) > 0)
    {
        // Check if there's already a query string
        fullEndpoint += (fullEndpoint.indexOf('?') == -1) ? "?" : "&";
        fullEndpoint += "sort=" + String(sort);
    }

    // Append the skipTotal parameter if provided
    if (skipTotal != nullptr && strlen(skipTotal) > 0)
    {
        // Check if there's already a query string
        fullEndpoint += (fullEndpoint.indexOf('?') == -1) ? "?" : "&";
        fullEndpoint += "skipTotal=" + String(skipTotal);
    }

    // Append the filter parameter if provided
    if (filter != nullptr && strlen(filter) > 0)
    {
        // Check if there's already a query string
        fullEndpoint += (fullEndpoint.indexOf('?') == -1) ? "?" : "&";
        fullEndpoint += "skipTotal=" + String(filter);
    }

    return main_connection.performGETRequest(fullEndpoint.c_str());
}

String PocketbaseArduino::deleteRecord(const char *recordId)
{
    String fullEndpoint = base_url + String(current_endpoint) + "records/" + recordId;

    return main_connection.performDELETERequest(fullEndpoint.c_str());
}

String PocketbaseArduino::create(const String &requestBody)
{
    // Construct the endpoint based on the current_endpoint
    String fullEndpoint = current_endpoint + "records/";

    // Call performPOSTRequest with the constructed endpoint and provided parameters
    return main_connection.performPOSTRequest(fullEndpoint.c_str(), requestBody);
}

void PocketbaseArduino::subscribe(
    const char *collection,
    const char *recordid,
    SubscriptionFn callback,
    void *ctx)
{
    if (subscription_count >= 5)
    {
        Serial.println("Maximum subscription count reached.");
        return;
    }

    int selected_index = -1;
    for (size_t i = 0; i < sizeof(subscription_ctx) / sizeof(subscription_ctx[0]); i++)
    {
        if (subscription_ctx[i].active == false)
        {
            selected_index = i;
            break;
        }
    }

    if (selected_index == -1)
    {
        Serial.println("No available subscription slot found.");
        return;
    }

    Serial.println("Subscribing to collection: " + String(collection) + ", recordid: " + String(recordid));

    subscription_ctx[selected_index].pb_connection = main_connection.fork();

    subscription_ctx[selected_index].collection = String(collection);
    subscription_ctx[selected_index].recordid = String(recordid);
    subscription_ctx[selected_index].callback = callback;
    subscription_ctx[selected_index].ctx = ctx;
    subscription_ctx[selected_index].active = true;
    subscription_ctx[selected_index].tcp_connection = new HTTPClient(); // Initialize the HTTPClient for this subscription

    auto &current = subscription_ctx[selected_index];
    subscription_count++;

    String url = base_url + "/realtime";

    auto https = current.tcp_connection;

    if (https->begin(*current.pb_connection.client, url))
    {
        https->addHeader("Accept", "text/event-stream");
        https->addHeader("Authorization", current.pb_connection.auth_token);
        https->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        // https->setReuse(true);
        // https->setTimeout(10000); // Set a timeout for the connection
        int code = https->GET();
        if (code > 0)
        {
            Serial.printf("[HTTPS] GET... code: %d\n", code);
            if (code == HTTP_CODE_OK || code == HTTP_CODE_NO_CONTENT)
            {
                Serial.println("[HTTPS] Connection established successfully.");
                //  current.pb_connection.client->setTimeout(10000); // Set a timeout for the client
                current.pb_connection.client->setNoDelay(true); // Disable Nagle's algorithm for better performance
                current.endpoint = url;
                // String payload = https->getString();
                // print request contents (must be removed)
                //  Serial.println(payload);
            }
            else
            {
                Serial.printf("[HTTPS] Error on connection: %s\n", https->errorToString(code).c_str());
            }
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect: %s\n", https->errorToString(code).c_str());
        }
        // https.end(); // End the HTTPS connection
        //  current.pb_connection.client->flush(); // Ensure the client is ready for reading
        // current.tcp_connection = std::move(https); // Store the HTTPClient in the subscription context
    }
    else
    {
        Serial.printf("[HTTPS] Unable to connect\n");
    }

    Serial.println("Subscription established for collection: " + String(collection) + ", recordid: " + String(recordid));

    // QNetworkRequest subscriptionRequest = QNetworkRequest(QUrl(url + "/api/realtime"));
    /// subscriptionRequest.setRawHeader("accept", "text/event-stream");
    // subscriptionRequest.setRawHeader("Authorization", token.toUtf8());
    // subscriptionRequest.setAttribute(QNetworkRequest::RedirectionTargetAttribute, true);
    // subscriptionRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
}

void PocketbaseArduino::update_subscription()
{
    for (size_t i = 0; i < sizeof(subscription_ctx) / sizeof(subscription_ctx[0]); i++)
    {
        if (subscription_ctx[i].active)
        {
            if (subscription_ctx[i].pb_connection.client == nullptr)
            {
                Serial.println("Subscription client is null, skipping update for collection: " + subscription_ctx[i].collection + ", recordid: " + subscription_ctx[i].recordid);
                continue;
            }

            auto &current = subscription_ctx[i];
            auto stream = current.tcp_connection->getStreamPtr();

            if (!stream->connected())
            {
                Serial.println("Subscription client is not connected, attempting to reconnect for collection: " + current.collection + ", recordid: " + current.recordid);
                current.tcp_connection->begin(*current.pb_connection.client, current.endpoint);
                current.tcp_connection->addHeader("Accept", "text/event-stream");
                current.tcp_connection->addHeader("Authorization", current.pb_connection.auth_token);
                current.tcp_connection->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
                int code = current.tcp_connection->GET();
                if (code <= 0)
                {
                    Serial.printf("[HTTPS] Unable to reconnect: %s\n", current.tcp_connection->errorToString(code).c_str());
                    continue;
                }
            }
            if (!stream->available())
            {
                continue;
            }
          //  String response = current.pb_connection.client->readStringUntil('\n');
         //   Serial.println("Received response: " + response);

            // Parse the JSON response

            String data;
            String event; 

            String line;
            // FIXME: extract this in another function
            while(stream->available())
            {
                
                char c = stream->read();
                
                if (c == '\n')
                {
                    if (!line.isEmpty())
                    {
                        // Process the line
                        Serial.println("Processing line: " + line);
                        if (line.startsWith("data:"))
                        {
                            data += line.substring(5); // Skip "data:"
                        }
                        else if (line.startsWith("event:"))
                        {
                            event = line.substring(6); // Skip "event:"
                        }
                        else 
                        {
                            Serial.println("Unknown line format: " + line);
                        }
                    }
                    line = ""; // Reset for the next line
                }
                else if(c != '\r')
                {
                    line += c; // Append character to the current line
                }
            }
            

            if(data.isEmpty() || event.isEmpty())
            {
                Serial.println("No valid data or event found in the response.");
                continue; // Skip processing if no valid data or event
            }
                // Call the callback function with the event and record
            current.callback(event, data, current.ctx);
            
        }
    }
}

void PocketbaseArduino::unsubscribe(
    const char *collection,
    const char *recordid)
{
    for (size_t i = 0; i < sizeof(subscription_ctx) / sizeof(subscription_ctx[0]); i++)
    {
        if (subscription_ctx[i].active &&
            subscription_ctx[i].collection == String(collection) &&
            subscription_ctx[i].recordid == String(recordid))
        {
            Serial.println("Unsubscribing from collection: " + String(collection) + ", recordid: " + String(recordid));
            subscription_ctx[i].active = false;
            subscription_ctx[i].pb_connection.client->stop(); // Stop the client connection
            delete subscription_ctx[i].pb_connection.client;  // Delete the client to free resources
            delete subscription_ctx[i].tcp_connection;        // Delete the HTTPClient to free resources
            subscription_count--;

            return;
        }
    }

    Serial.println("No active subscription found for collection: " + String(collection) + ", recordid: " + String(recordid));
}