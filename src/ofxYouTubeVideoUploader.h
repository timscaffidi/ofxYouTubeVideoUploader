//
//  ofxYouTubeVideoUploader.h
//  videoUploader
//
//  Created by Timothy Scaffidi on 11/27/12. for Helios Interactive
//
//
#pragma once

#include <string>
#include "ofxSSL.h"
#include "ofxJSONElement.h"
#include "ofMain.h"

struct ofxYouTubeOAuthInfo {
    string dev_key;
    string client_id;
    string client_secret;
    string grant_type;
    string scope;
    string device_code;
    string user_code;
    string verification_url;
    int expires_in;
    int interval;
    string access_token;
    string token_type;
    string refresh_token;
    unsigned long long access_time;
    bool bIsAuthorized;
    bool bIsPolling;
    string jsonFile;
    ofxYouTubeOAuthInfo() {
        jsonFile = "oAuthSettings.json";
    }
    void setFromJSON(ofxJSONElement json){
        dev_key = json.get("dev_key", "").asString();
        client_id = json.get("client_id", "").asString();
        client_secret = json.get("client_secret", "").asString();
        grant_type = json.get("grant_type", "http://oauth.net/grant_type/device/1.0").asString();
        scope = json.get("scope", "https://uploads.gdata.youtube.com").asString();
        device_code = json.get("device_code", "").asString();
        user_code = json.get("user_code", "").asString();
        verification_url = json.get("verification_url", "").asString();
        expires_in = json.get("expires_in", "0").asInt();
        interval = json.get("interval", "0").asInt();
        access_token = json.get("access_token", "").asString();
        token_type = json.get("token_type", "").asString();
        refresh_token = json.get("refresh_token", "").asString();
        jsonFile = json.get("jsonFile", "oAuthSettings.json").asString();
        bIsAuthorized = false;
        bIsPolling = false;
        access_time = 0;
    }
    ofxJSONElement getJSONElement(){
        ofxJSONElement json;
        json["dev_key"] = dev_key;
        json["client_id"] = client_id;
        json["client_secret"] = client_secret;
        json["grant_type"] = grant_type;
        json["scope"] = scope;
        json["device_code"] = device_code;
        json["user_code"] = user_code;
        json["verification_url"] = verification_url;
        json["expires_in"] = expires_in;
        json["interval"] = interval;
        json["access_token"] = access_token;
        json["token_type"] = token_type;
        json["refresh_token"] = refresh_token;
        json["jsonFile"] = jsonFile;
        return json;
    }
};

class ofxYouTubeVideoUploader : private ofThread {
public:
    enum uploadStatus {
        UPLOAD_FAILED,
        UPLOAD_NOT_STARTED,
        UPLOADING,
        UPLOAD_SUCCESS
    };
    ofxYouTubeVideoUploader();
    ~ofxYouTubeVideoUploader();
    void setup(string settingsFileName);
    void setup(ofxYouTubeOAuthInfo info);
    
    void uploadVideoFile(string path, string fileName, string title="", string description="", string keywords="");
    bool isWaitingForAuthorization() { return authInfo.bIsPolling; }
    string getUserCode() { return authInfo.user_code; }
    string getVerificationUrl() { return authInfo.verification_url; }
    bool isAuthorized() { return authInfo.bIsAuthorized; }
    ofxYouTubeOAuthInfo getAuthInfo() { return authInfo; }
    uploadStatus getUploadStatus() { return mUploadStatus; }
    string getUploadedVideoURL() { return mUploadedURL; }
    bool launchBrowser;
private:
    void threadedFunction();
    void setup();
    string requestAccess();
    bool pollAccessServer();
    bool refreshAccess();
    
    uploadStatus mUploadStatus;
    string mUploadedURL;
    
    string getMetaDataXMLString(string title, string description, string keywords);

    ofxYouTubeOAuthInfo authInfo;
    ofxSSL curl;
    unsigned long long lastPollTime;
};