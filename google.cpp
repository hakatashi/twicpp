#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include "oauth.h"
#include "key.h"
#include "picojson.h"
#include "babel/babel.h"

#include "curl/curl.h"

size_t callBackFunk(char* ptr, size_t size, size_t nmemb, std::string* stream)
{
    int realsize = size * nmemb;
    stream->append(ptr, realsize);
    return realsize;
}

int DebugFunk(CURL* curl, curl_infotype info, char* ptr, size_t size, std::string* stream)
{
    int realsize = size;
    stream->append(ptr, realsize);
    return 0;
}

int func_google(std::vector<std::string> *item, std::string *result) {
	CURL *access;
	CURLcode res;
	std::string url,urlen;
	char *encoded;
	const char *error;
	std::string reply,head,jisrep;
	struct curl_slist *slist=NULL;

	access = curl_easy_init();

	encoded = curl_easy_escape(access, (*item)[1].c_str(), 0);
	urlen.assign(encoded);
	url = "http://ajax.googleapis.com/ajax/services/search/web?v=1.0&q=" + urlen;

	curl_easy_setopt(access, CURLOPT_URL, url.c_str());
	curl_easy_setopt(access, CURLOPT_WRITEFUNCTION, callBackFunk);
	curl_easy_setopt(access, CURLOPT_WRITEDATA, (std::string*)&reply);
	curl_easy_setopt(access, CURLOPT_DEBUGFUNCTION, DebugFunk);
	curl_easy_setopt(access, CURLOPT_DEBUGDATA, (std::string*)&head);
	slist = curl_slist_append(slist, "Accept: text/javascript");
	slist = curl_slist_append(slist, "Accept-Language: ja");
	curl_easy_setopt(access, CURLOPT_HTTPHEADER, slist);
	res = curl_easy_perform(access);
	curl_easy_cleanup(access);
	curl_slist_free_all(slist);

	curl_free(encoded);

	if (res != CURLE_OK) {
		error=curl_easy_strerror(res);
		return 1;
	}

	jisrep = babel::utf8_to_sjis(reply);

	printf("%s", jisrep.c_str());

	picojson::value v;

	const char *string = jisrep.c_str();
	picojson::parse(v, string, string + strlen(string));

	picojson::array arr = v.get<picojson::object>()["responseData"].get<picojson::object>()["results"].get<picojson::array>();

	if (arr.size()!=0) {
		for (picojson::array::const_iterator it = arr.begin(); it != arr.end(); ++it) {
			picojson::object obj = it->get<picojson::object>();
			if (it != arr.begin()) *result += "\n";
			*result += obj["unescapedUrl"].to_str();
		}
	}

	return 0;
}