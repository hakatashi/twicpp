/**
 *  @brief example code for liboauth using http://term.ie/oauth/example
 *  @file oauthexample.c
 *  @author Robin Gareus <robin@gareus.org>
 *
 * Copyright 2008 Robin Gareus <robin@gareus.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oauth.h"

#include <vector>
#include <string>

#include <shlobj.h>
#include "curl/curl.h"

#include "picojson.h"
#include "key.h"
#include "babel/babel.h"

int func_fav(std::vector<std::string> *item, int *num);
int func_yomi(std::vector<std::string> *item, std::string *yomi);
int func_google(std::vector<std::string> *item, std::string *result);

//int func_fav(std::vector<std::string> *item);

/**
 * split and parse URL parameters replied by the test-server
 * into <em>oauth_token</em> and <em>oauth_token_secret</em>.
 */

bool parse_reply(const char *reply, std::string *token, std::string *secret)
{
	std::vector<std::string> arr;

	char const *end = reply + strlen(reply);
	char const *left = reply;
	char const *right = left;
	while (1) {
		int c = 0;
		if (right < end) {
			c = *right;
		}
		if (c == 0 || c == '&') {
			std::string str(left, right);
			arr.push_back(str);
			if (c == 0) {
				break;
			}
			right++;
			left = right;
		}
		right++;
	}

	char const *oauth_token = 0;
	char const *oauth_token_secret = 0;

	for (std::vector<std::string>::const_iterator it = arr.begin(); it != arr.end(); it++) {
		if (strncmp(it->c_str(), "oauth_token=", 12) == 0) {
			oauth_token = it->c_str() + 12;
		} else if (strncmp(it->c_str(), "oauth_token_secret=", 19) == 0) {
			oauth_token_secret = it->c_str() + 19;
		}
	}

	if (oauth_token && oauth_token_secret) {
		if (token) {
			*token = oauth_token;
		}
		if (secret) {
			*secret = oauth_token_secret;
		}
		return true;
	}

	return false;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

std::string c_key; //< consumer key
std::string c_secret; //< consumer secret

std::string t_key;
std::string t_secret;

std::string yahoo_id;

std::string inputtext()
{
	char tmp[100];
	fgets(tmp, 100, stdin);
	size_t i = strlen(tmp);
	while (i > 0 && (tmp[i - 1] == '\n' || tmp[i - 1] == '\r')) {
		i--;
	}
	return std::string(tmp, tmp + i);
}

std::string tweet_gen(std::string message, long long int in_reply_to) {
	std::string req_url;
	std::string postarg;
	std::string reply;
	char temp[64];

	sprintf(temp,"%03d",rand()%1000);
	message += "(pid=";
	message += temp ;
	message += ")";

	if (!message.empty()) {
		{
			int n;
			wchar_t ucs2[1000];
			char utf8[1000];
			n = MultiByteToWideChar(CP_ACP, 0, message.c_str(), message.size(), ucs2, 1000);
			n = WideCharToMultiByte(CP_UTF8, 0, ucs2, n, utf8, 1000, 0, 0);
			message.assign(utf8, n);
		}

//		std::string uri = "http://api.twitter.com/statuses/update.xml";
		std::string uri = "http://api.twitter.com/1.1/statuses/update.json"; // 2012-11-15
		uri += "?status=";
		uri += oauth_url_escape(message.c_str());
		uri += "&in_reply_to_status_id=";
		sprintf(temp,"%lld",in_reply_to);
		uri += temp;

		req_url = oauth_sign_url2(uri.c_str(), &postarg, OA_HMAC, 0, c_key.c_str(), c_secret.c_str(), t_key.c_str(), t_secret.c_str());
		reply = oauth_http_post(req_url.c_str(), postarg.c_str(), false);

		return reply;
	}
}

std::vector<std::string> split(std::string str, std::string delim) {
  std::vector<std::string> items;
  std::size_t dlm_idx;
  if(str.npos == (dlm_idx = str.find_first_of(delim))) {
    items.push_back(str.substr(0, dlm_idx));
  }
  while(str.npos != (dlm_idx = str.find_first_of(delim))) {
    if(str.npos == str.find_first_not_of(delim)) {
      break;
    }
    items.push_back(str.substr(0, dlm_idx));
    dlm_idx++;
    str = str.erase(0, dlm_idx);
    if(str.npos == str.find_first_of(delim) && "" != str) {
      items.push_back(str);
      break;
    }
  }
  return items;
}

//static char const consumer_key[] = "xxxxxxxxxxxxxxxxxxxxxx";
//static char const consumer_secret[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
#include "key.txt" // Å©Ç±ÇÃÉtÉ@ÉCÉãÇÕîÈñßÇ≈Ç∑ÅBÅ™ÇÃólÇ»ì‡óeÇ™èëÇ©ÇÍÇΩÇæÇØÇÃÉtÉ@ÉCÉãÇ≈Ç∑ÅB


#include "socket.h"

void twitter_example()
{
	static const char *request_token_uri = "http://api.twitter.com/oauth/request_token";
	static const char *authorize_uri = "http://api.twitter.com/oauth/authorize";
	static const char *access_token_uri = "http://api.twitter.com/oauth/access_token";

	std::string req_url;
	std::string postarg;
	std::string reply;

	long long int since=0;

#if 0

	//

	c_key = consumer_key;
	c_secret = consumer_secret;

	//

	printf("Request token..\n");
	
	{
		std::string reply;
		std::string req_url = oauth_sign_url2(request_token_uri, &postarg, OA_HMAC, 0, c_key.c_str(), c_secret.c_str(), 0, 0);
		reply = oauth_http_post(req_url.c_str(),postarg.c_str(), false);

		if (!parse_reply(reply.c_str(), &t_key, &t_secret)) {
			throw "failed to get request token.";
		}
	}

	//

	printf("Authorize..\n");

	{
		std::string req_url = oauth_sign_url2(authorize_uri, 0, OA_HMAC, 0, c_key.c_str(), c_secret.c_str(), t_key.c_str(), t_secret.c_str());

		printf("Opening...\n");
		puts(req_url.c_str());

		ShellExecute(0, 0, req_url.c_str(), 0, 0, SW_SHOW); // ÉEÉFÉuÉuÉâÉEÉUÇãNìÆÇ∑ÇÈ
	}

	//

	{
		printf("Input PIN: ");
		std::string pin = inputtext();
		putchar('\n');

		printf("Access token..\n");

		std::string url = access_token_uri;
		url += "?oauth_verifier=";
		url += pin;

		std::string req_url = oauth_sign_url2(url.c_str(), 0, OA_HMAC, 0, c_key.c_str(), 0, t_key.c_str(), 0);
		std::string reply = oauth_http_get(req_url.c_str(), postarg.c_str());

		if (!parse_reply(reply.c_str(), &t_key, &t_secret)) {
			throw "failed to get access token.";
		}
	}

	// now retrieved 't_key' is access token and 't_secret' is access secret.

	printf("access key: %s\n", t_key.c_str());
	printf("access secret: %s\n", t_secret.c_str());
#else
    c_key = "QMhh0Piu1SPtf3xZWWUgg";
    c_secret = "7XmvKrzLD9L4IYUgovxD5cVaJP5kOnfPzciieXcH9k0";
    t_key = "1026378019-v3zdkZ3H339q1DR6sDhVVWdT98w35YtL4n7csTH";
    t_secret = "QDy6gJEEn6KQB5Iv3HddaXpp6Tde222D16UKnuRGeo";
	yahoo_id = "dj0zaiZpPUhtZmxiRDFoVEtxMSZkPVlXazlaa3BPYzBRM05Ha21jR285TUEtLSZzPWNvbnN1bWVyc2VjcmV0Jng9NTI-";
#endif
	// call Twitter API

	/*printf("make some request..\n");

	printf("input message: ");
	std::string message = "@hakatashi Ç±ÇÒÇŒÇÒÇÕ";//inputtext();
	putchar('\n');
	if (!message.empty()) {
		{
			int n;
			wchar_t ucs2[1000];
			char utf8[1000];
			n = MultiByteToWideChar(CP_ACP, 0, message.c_str(), message.size(), ucs2, 1000);
			n = WideCharToMultiByte(CP_UTF8, 0, ucs2, n, utf8, 1000, 0, 0);
			message.assign(utf8, n);
		}

//		std::string uri = "http://api.twitter.com/statuses/update.xml";
		std::string uri = "http://api.twitter.com/1/statuses/update.xml"; // 2012-11-15
		uri += "?status=";
		uri += oauth_url_escape(message.c_str());

		req_url = oauth_sign_url2(uri.c_str(), &postarg, OA_HMAC, 0, c_key.c_str(), c_secret.c_str(), t_key.c_str(), t_secret.c_str());
		reply = oauth_http_post(req_url.c_str(), postarg.c_str(), false);
	}*/

	{
		FILE *sincef;
		sincef=fopen("since.txt","r");
		fscanf(sincef,"%lld",&since);
		fclose(sincef);
	}

	while(1) {
		std::string uri = "http://api.twitter.com/1.1/statuses/mentions_timeline.json?count=200";

		uri += "&since_id=";
		char temp[64];
		sprintf(temp,"%lld",since);
		uri += temp;

		printf("request URL: %s\n", uri.c_str());

		printf("making request...");

		req_url = oauth_sign_url2(uri.c_str(), 0, OA_HMAC, 0, c_key.c_str(), c_secret.c_str(), t_key.c_str(), t_secret.c_str());
		reply = oauth_http_get(req_url.c_str(), postarg.c_str());
		
		puts("completed");

		printf("parsing...");

		picojson::value v;

		const char *string = reply.c_str();
		picojson::parse(v, string, string + strlen(string));

		puts("completed");

		picojson::array arr = v.get<picojson::array>();

		if (arr.size()!=0) {
			for (picojson::array::const_iterator it = arr.begin(); it != arr.end(); ++it) {
				picojson::object obj = it->get<picojson::object>();
				std::string com = obj["text"].to_str();
				std::string user = obj["user"].get<picojson::object>()["screen_name"].to_str();
				std::vector<std::string> item = split(com, " ");
				long long int twiid = _atoi64(obj["id_str"].to_str().c_str());
				if (since<twiid) {
					since=twiid;
				}
				for (std::vector<std::string>::iterator it = item.begin() ; it != item.end() ; ++it ) {
					if (!strcmp(it->c_str(),"@HKTS_maid")) {
						it = item.erase(it);
						if (it==item.end()) break;
					}
				}
				if (item.size()!=0) {
					if (!strcmp(item[0].c_str(),"fav")) {
						if (item.size()>1) {
							std::string tweets = "@" + user + " " + "ÅyñΩóﬂéÛóùÅzóπâÇ¢ÇΩÇµÇ‹ÇµÇΩÅB " + item[1] + " ólÇÇ¢Ç¡ÇœÇ¢Ç”ÇüÇ⁄Ç”ÇüÇ⁄ívÇµÇ‹Ç∑ÅB";
							tweet_gen(tweets,twiid);
							int num=0;
							char temp[64];
							switch (func_fav(&item, &num)) {
							case 0:
								sprintf(temp,"%d",num);
								tweets = "@" + user + " " + "ÅyêãçsïÒçêÅz " + item[1] + " ólÇ" + temp + "âÒÇŸÇ«Ç”ÇüÇ⁄Ç”ÇüÇ⁄ívÇµÇ‹ÇµÇΩÅB " + item[1] + " ólÇ‡Ç≥ÇºÇ©ÇµÇ®äÏÇ—Ç≈ÇµÇÂÇ§ÅB";
								tweet_gen(tweets,twiid);
								break;
							case 1:
								tweets = "@" + user + " " + "Åyêãçsé∏îsÅzê\ÇµñÛÇ≤Ç¥Ç¢Ç‹ÇπÇÒÅB " + item[1] + " ólÇÕåªç›Ç¢ÇÁÇ¡ÇµÇ·ÇÁÇ»Ç¢ÇÊÇ§Ç≈Ç∑ÅB";
								tweet_gen(tweets,twiid);
								break;
							}
						} else {
							std::string tweets = "@" + user + " " + "Åyì`íBé∏îsÅzÇ«ÇøÇÁólÇÇ”ÇüÇ⁄Ç”ÇüÇ⁄Ç∑ÇÍÇŒÇÊÇ¢ÇÃÇ≈ÇµÇÂÇ§Ç©ÅB";
							tweet_gen(tweets,twiid);
						}
					} else if (!strcmp(item[0].c_str(),"yomi")) {
						if (item.size()>1) {
							std::string tweets;
							int num=0;
							std::string kana;
							char temp[64];
							switch (func_yomi(&item, &kana)) {
							case 0:
								sprintf(temp,"%d",num);
								tweets = "@" + user + " " + "ÅyêãçsïÒçêÅzÅu" + kana + "ÅvÇ≈å‰ç¿Ç¢Ç‹Ç∑ÅB";
								tweet_gen(tweets,twiid);
								break;
							}
						} else {
							std::string tweets = "@" + user + " " + "Åyì`íBé∏îsÅzÇ«ÇÃäøéöÇÃì«Ç›ÇÇ®ã≥Ç¶Ç∑ÇÍÇŒÇÊÇÎÇµÇ¢ÇÃÇ≈ÇµÇÂÇ§Ç©ÅB";
							tweet_gen(tweets,twiid);
						}
					} else if (!strcmp(item[0].c_str(),"google")) {
						if (item.size()>1) {
							std::string tweets,result;
							switch (func_google(&item,&result)) {
							case 0:
								tweets = "@" + user + " " + "ÅyêãçsïÒçêÅzà»â∫Ç™åãâ Ç≈Ç∑ÅB\n" + result + "\n";
								tweet_gen(tweets,twiid);
								break;
							case 1:
								tweets = "@" + user + " " + "Åyêãçsé∏îsÅzê\ÇµñÛÇ≤Ç¥Ç¢Ç‹ÇπÇÒÅBcurlÇÃÉGÉâÅ[Ç≈Ç∑ÅB";
								tweet_gen(tweets,twiid);
								break;
							}
						} else {
							std::string tweets = "@" + user + " " + "Åyì`íBé∏îsÅzÇ«ÇÃÇÊÇ§Ç»ì‡óeÇ≈ÉOÉOÇÍÇŒÇÊÇÎÇµÇ¢ÇÃÇ≈ÇµÇÂÇ§Ç©ÅB";
							tweet_gen(tweets,twiid);
						}
					} else {
						std::string tweets = "@" + user + " " + "Åyì`íBé∏îsÅzÇªÇÃÇÊÇ§Ç»ñΩóﬂÇÕÇΩÇæÇ¢Ç‹è≥Ç¡ÇƒÇ®ÇËÇ‹ÇπÇÒÅBç\ï∂Ç…âàÇ¡ÇƒÇ‡Ç§àÍìxñΩóﬂÇåæÇ¢Ç¬ÇØÇƒÇ≠ÇæÇ≥Ç¢Ç‹ÇµÅB";
						tweet_gen(tweets,twiid);
					}
				} else {
					std::string tweets = "@" + user + " " + "Åyì`íBé∏îsÅzÇ¢Ç©Ç™Ç»Ç≥Ç¢Ç‹ÇµÇΩÇ©ÅAÇ≤éÂêlÇ≥Ç‹ÅH";
					tweet_gen(tweets,twiid);
				}
			}
		}

		FILE *sincef;
		sincef = fopen("since.txt", "w");
		fprintf(sincef,"%lld\n",since);
		fclose(sincef);
		
		Sleep(60000);
	}
}

int main (int argc, char **argv)
{
	babel::init_babel();
	srand((unsigned) time(NULL));
	try {
		Socket::initialize();
		twitter_example();
	} catch (char const *error) {
		puts(error);
	}
	return 0;
}
