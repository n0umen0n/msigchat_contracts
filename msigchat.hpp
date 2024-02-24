
#include <eosio/eosio.hpp>
#include <vector>
#include <string>
#include <map>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <numeric>

using namespace eosio;
using namespace std;


CONTRACT msigchat : public contract
{
public:

    TABLE chats {
        name chat_account;
        string permission;
        string description;
        string community_profile_img_url;
        string community_background_img_url;

        uint64_t primary_key() const { return chat_account.value; }
    };
    typedef multi_index<"chats"_n, chats> chats_table;

    TABLE profile {
        name user;
        string name_in_chat;
        string description;
        string profile_img_url;

        uint64_t primary_key() const { return user.value; }
    };
    typedef multi_index<"profile"_n, profile> profile_table;


    TABLE delapproval {
        name chat_account;
        bool approved_to_delete;

        uint64_t primary_key() const { return chat_account.value; }
    };
    typedef multi_index<"delapproval"_n, delapproval> delapproval_table;
    msigchat(name self, name code, datastream<const char *> ds);

    ACTION setprofile(name user, string name_in_chat, string description, string profile_img_url);
    ACTION delprofile(name user);
    ACTION setchat(name adder, name chat_account, string permission, string description, string community_profile_img_url, string community_background_img_url);
    ACTION delchat(name chat_account);
    ACTION sendmessage(string message, name user, name chat_account);
    ACTION delmessage(name user, uint64_t message_id, name chat_account);
    ACTION delmessages(name chat_account, uint16_t number_of_messages);
    ACTION deloffon(name chat_account, bool delon);


private:

void require_auth_either(name user1, name user2);


};
