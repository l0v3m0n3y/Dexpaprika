#ifndef DEXPAPRIKA_H
#define DEXPAPRIKA_H

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <pplx/pplx.h>
#include <iostream>
#include <string>
#include <vector>

using namespace web;
using namespace web::http;
using namespace web::http::client;

class Dexpaprika {
private:
    std::string api_base = "https://api.dexpaprika.com";
    http_client_config client_config;
    
    http_request create_request(const std::string& endpoint, const std::string& method, const std::string& data = "") {
        http_request request;
        
        if (method == "GET") {
            request.set_method(methods::GET);
        } else if (method == "POST") {
            request.set_method(methods::POST);
        } else if (method == "PUT") {
            request.set_method(methods::PUT);
        } else if (method == "DEL") {
            request.set_method(methods::DEL);
        }
        
        request.set_request_uri(utility::conversions::to_string_t(endpoint));
        
        // Set headers
        request.headers().add(U("Host"), U("api.dexpaprika.com"));
        request.headers().add(U("User-Agent"), U("Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0"));
        
        if (!data.empty() && (method == "POST" || method == "PUT")) {
            request.set_body(data);
        }
        
        return request;
    }
    
    std::string build_query_params(const std::map<std::string, std::string>& params) {
        if (params.empty()) return "";
        
        std::string query = "?";
        bool first = true;
        for (const auto& param : params) {
            if (!param.second.empty()) {
                if (!first) query += "&";
                // Кодируем значения параметров
                auto encoded_value = web::uri::encode_data_string(utility::conversions::to_string_t(param.second));
                query += param.first + "=" + utility::conversions::to_utf8string(encoded_value);
                first = false;
            }
        }
        return query;
    }
    
    pplx::task<json::value> make_api_call(const std::string& endpoint, const std::string& method, const std::string& data = "") {
        http_client client(utility::conversions::to_string_t(api_base), client_config);
        auto request = create_request(endpoint, method, data);

        return client.request(request)
            .then([](http_response response) {
                if (response.status_code() == status_codes::OK) {
                    return response.extract_json();
                } else {
                    json::value error_obj;
                    error_obj[U("error")] = json::value::string(
                        U("HTTP Error: ") + utility::conversions::to_string_t(std::to_string(response.status_code())));
                    error_obj[U("success")] = json::value::boolean(false);
                    return pplx::task_from_result(error_obj);
                }
            })
            .then([](pplx::task<json::value> previousTask) {
                try {
                    return previousTask.get();
                } catch (const std::exception& e) {
                    json::value error_obj;
                    error_obj[U("error")] = json::value::string(
                        U("Exception: ") + utility::conversions::to_string_t(e.what()));
                    error_obj[U("success")] = json::value::boolean(false);
                    return error_obj;
                }
            });
    }

public:
    Dexpaprika(){
        client_config.set_validate_certificates(false);
    }

    pplx::task<json::value> networks_list() {
        return make_api_call("/networks","GET");
    }

    pplx::task<json::value> search(const std::string& query) {
        return make_api_call("/search?query=" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(query))),"GET");
    }

    pplx::task<json::value> get_top_pools_for_token(const std::string& networks,const std::string& token_address,const std::string& sort="",const std::string& order_by="",const std::string& reorder="",const std::string& address="") {
        std::map<std::string, std::string> params;
        if (!sort.empty()) params["sort"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(sort)));
        if (!order_by.empty()) params["order_by"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(order_by)));
        if (!reorder.empty()) params["reorder"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(reorder)));
        if (!address.empty()) params["address"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(address)));
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/tokens/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(token_address))) + "/pools" + build_query_params(params),"GET");
    }

    pplx::task<json::value> get_networks_multi_prices(const std::string& networks,const std::string& tokens="") {
        std::map<std::string, std::string> params;
        if (!tokens.empty()) params["tokens"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(tokens)));
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/multi/prices" + build_query_params(params),"GET");
    }

    pplx::task<json::value> get_latest_data_on_network(const std::string& networks,const std::string& token_address,int limit=10) {
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/tokens/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(token_address))) + "/transactions?limit=" + std::to_string(limit),"GET");
    }

    pplx::task<json::value> get_transactions_on_pool(const std::string& networks,const std::string& token_address,int limit=10,int page=1,const std::string& cursor="") {
        std::map<std::string, std::string> params;
        if (limit>0) params["limit"] = std::to_string(limit);
        if (page>0) params["page"] = std::to_string(page);
        if (!cursor.empty()) params["cursor"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(cursor)));
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/tokens/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(token_address))) + "/transactions" + build_query_params(params),"GET");
    }

    pplx::task<json::value> get_ohlcv_on_pool(const std::string& networks,const std::string& pool_address,int limit=10,const std::string& interval="24h",bool inversed=false) {
        std::map<std::string, std::string> params;
        if (limit>0) params["limit"] = std::to_string(limit);
        if (!interval.empty()) params["interval"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(interval)));
        params["inversed"] = inversed;
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/pools/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(pool_address))) + "/ohlcv" + build_query_params(params),"GET");
    }

    pplx::task<json::value> get_pool_on_network(const std::string& networks,const std::string& pool_address,bool inversed=false) {
        std::map<std::string, std::string> params;
        params["inversed"] = inversed;
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/pools/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(pool_address))) + build_query_params(params),"GET");
    }

    pplx::task<json::value> get_top_dexes_on_network(const std::string& networks,const std::string& dexes,int limit=0,int page=1,const std::string& sort="",const std::string& order_by="") {
        std::map<std::string, std::string> params;
        if (limit>0) params["limit"] = std::to_string(limit);
        if (page>0) params["page"] = std::to_string(page);
        if (!sort.empty()) params["sort"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(sort)));
        if (!order_by.empty()) params["order_by"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(order_by)));
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/dexes/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(dexes))) + "/pools" + build_query_params(params),"GET");
    }

    pplx::task<json::value> get_top_pools(const std::string& networks,int limit=0,int page=1,const std::string& sort="",const std::string& order_by="") {
        std::map<std::string, std::string> params;
        if (limit>0) params["limit"] = std::to_string(limit);
        if (page>0) params["page"] = std::to_string(page);
        if (!sort.empty()) params["sort"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(sort)));
        if (!order_by.empty()) params["order_by"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(order_by)));
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/pools" + build_query_params(params),"GET");
    }

    pplx::task<json::value> get_top_dexes(const std::string& networks,int limit=0,int page=1,const std::string& sort="",const std::string& order_by="") {
        std::map<std::string, std::string> params;
        if (limit>0) params["limit"] = std::to_string(limit);
        if (page>0) params["page"] = std::to_string(page);
        if (!sort.empty()) params["sort"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(sort)));
        if (!order_by.empty()) params["order_by"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(order_by)));
        return make_api_call("/networks/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(networks))) + "/dexes" + build_query_params(params),"GET");
    }

};

#endif

