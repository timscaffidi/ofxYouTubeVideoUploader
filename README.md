#Usage:
1. Set up a developer account with google: https://developers.google.com/youtube/2.0/developers_guide_protocol_oauth2#OAuth2_Register
2. Read the oAuth2 [device flow](https://developers.google.com/youtube/2.0/developers_guide_protocol_oauth2#OAuth2_Devices_Flow)
3. populate a json file in your data folder with your client_id, client_secret, and dev_key. See the example.

The json file with eventually contain your access_key and refresh_key, but for the first use, you will need to authorize your oF app and grant it access to a youtube account. By default the addon will launch a browser window for you to type in the user code, you can find the user code in the console output or get it from the uploader object.
Once the app has been authenticated, it will periodically refresh its access, if you restart the app it will use the saved refresh_key to attempt authorization without a login step.

**Requires modified ofxSSL and ofxJSON**
https://github.com/timscaffidi/ofxJSON.git
https://github.com/timscaffidi/ofxSSL.git

created by [Timothy Scaffidi](https://github.com/timscaffidi) for [Helios Interactive](https://github.com/heliosinteractive)