#include <iostream>
#include <stdio.h>
#include <vector>
#include "oauth.h"
#include "key.h"
#include "picojson.h"

int func_fav(std::vector<std::string> *item, int *num) {
	std::string req_url, reply, postarg;
	long long int processed=0;
	int favd=0;
	int favnum;

	std::string name((*item)[1]);
	if (name.at(0) == '@') name.erase(name.begin());

	if ((*item).size()<=2) {
		favnum=200;
	} else {
		favnum=atoi((*item)[2].c_str());
	}

	if (favnum==0) favnum=200;
	
	std::string uri = "http://api.twitter.com/1.1/users/lookup.json";
	uri += "?screen_name=";
	uri += name;

	req_url = oauth_sign_url2(uri.c_str(), 0, OA_HMAC, 0, c_key.c_str(), c_secret.c_str(), t_key.c_str(), t_secret.c_str());
	reply = oauth_http_get(req_url.c_str(), postarg.c_str());

	if (reply.size()<5) {
		return 1;
	}

	picojson::value v;
	const char *temp;
	char temp2[64];

	while(favd<favnum) {
		uri = "http://api.twitter.com/1.1/statuses/user_timeline.json";
		uri += "?screen_name=";
		uri += name;
		uri += "&include_rts=false";
		uri += "&count=200";
		if (processed!=0) {
			uri += "&max_id=";
			sprintf(temp2,"%lld",processed-1);
			uri += temp2;
		}

		req_url = oauth_sign_url2(uri.c_str(), 0, OA_HMAC, 0, c_key.c_str(), c_secret.c_str(), t_key.c_str(), t_secret.c_str());
		reply = oauth_http_get(req_url.c_str(), postarg.c_str());

		temp = reply.c_str();
		picojson::parse( v, temp, temp + strlen(temp));

		picojson::array arr = v.get<picojson::array>();

		if (arr.size()==0) break;

		for (picojson::array::const_iterator it = arr.begin(); it != arr.end(); ++it) {
			picojson::object obj = it->get<picojson::object>();

			if (obj["favorited"].get<bool>()==false) {
				uri = "http://api.twitter.com/1.1/favorites/create.json";
				uri += "?id=";
				uri += obj["id_str"].to_str();

				req_url = oauth_sign_url2(uri.c_str(), &postarg, OA_HMAC, 0, c_key.c_str(), c_secret.c_str(), t_key.c_str(), t_secret.c_str());
				reply = oauth_http_post(req_url.c_str(), postarg.c_str(), false);

				int k;
				temp = reply.c_str();
				picojson::parse( v, temp, temp + strlen(temp));
				if ((k=v.get<picojson::object>().size())>0) {
					favd++;
					printf("%d fav\n",favd);
				}
			}

			processed=_atoi64(obj["id_str"].to_str().c_str());
		}
	}

	*num=favd;

	return 0;
}