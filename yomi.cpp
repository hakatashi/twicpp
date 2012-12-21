#include <iostream>
#include <stdio.h>
#include <vector>
#include "oauth.h"
#include "key.h"
#include "picojson.h"
#include "babel/babel.h"

int func_yomi(std::vector<std::string> *item, std::string *yomi) {
	std::string req_url, reply, postarg, kana, jisrep;

	req_url = "http://kudaranai.jp/mecab/mj.cgi?s=";
	req_url += (*item)[1];
	reply = oauth_http_get(req_url.c_str(), postarg.c_str());

	picojson::value v;

	jisrep = babel::utf8_to_sjis(reply);

	const char *temp = jisrep.c_str();

	picojson::parse(v, temp, temp+strlen(temp));

	picojson::array arr = v.get<picojson::array>();

	kana = "";

	for (picojson::array::const_iterator it = arr.begin(); it != arr.end(); ++it) {
		picojson::object obj = it->get<picojson::object>();
		if (obj.size()==2) {
			if (obj["feature"].get<picojson::array>().size()>=8) {
				kana += obj["feature"].get<picojson::array>()[7].to_str();
			} else {
				kana += obj["surface"].to_str();
			}
		} else continue;
	}

	*yomi = kana;

	return 0;
}