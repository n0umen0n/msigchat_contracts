
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


//PHASE 1 TABLE

    TABLE chats {
        name chat_account;
        string permission;
        string description;
        string community_profile_img_url;
        string community_background_img_url;

        uint64_t primary_key() const { return chat_account.value; }
    };
    typedef multi_index<"chats"_n, chats> chats_table;


    TABLE messagestb {
        uint64_t id;
        string message;
        name user;
        time_point_sec message_time;

        uint64_t primary_key() const { return id; }
    };
    typedef multi_index<"messagestb"_n, messagestb> messages_tb;


    TABLE chats1 {
        name chat_account;
        string permission;
        string description;
        string community_profile_img_url;
        string community_background_img_url;
        string community_name;

        uint64_t primary_key() const { return chat_account.value; }
    };
    typedef multi_index<"chats1"_n, chats1> chats1_table;


    struct emoji_reaction {
        uint16_t emoji_id;
        uint64_t count;
        vector<name> users; // Added to store the users who left the emoji
    };


    TABLE messagestb2 {
        uint64_t id;
        string message;
        name user;
        time_point_sec message_time;
        vector<emoji_reaction> emojis;
        string replied_to;

        uint64_t primary_key() const { return id; }
    };
    typedef multi_index<"messagestb2"_n, messagestb2> messages2_tb;

    TABLE profile {
        name user;
        string name_in_chat;
        string description;
        string profile_img_url;

        uint64_t primary_key() const { return user.value; }
    };
    typedef multi_index<"profile"_n, profile> profile_table;



    TABLE locproposal1 {
            name proposal_name;
            std::vector<char> packed_transaction;
            string title;
            string description;
            name community;
            time_point_sec proposal_time;
            name proposer;
            name creator;
            bool executed;

            uint64_t primary_key()const { return proposal_name.value; }
         };

    typedef multi_index< "locproposal1"_n, locproposal1 > proposals_tb;

    TABLE pinmessage5 {
            uint64_t message_id;
            name community;
            string pinned_message;
            name user_who_pinned;
            name user_who_created_message;
            time_point_sec message_time;
            time_point_sec pin_time;
            vector<emoji_reaction> emojis;

            uint64_t primary_key()const { return message_id; }
         };

    typedef multi_index< "pinmessage5"_n, pinmessage5 > pinmessage_tb;

    TABLE delapproval {
        name chat_account;
        bool approved_to_delete;

        uint64_t primary_key() const { return chat_account.value; }
    };
    typedef multi_index<"delapproval"_n, delapproval> delapproval_table;
    
    msigchat(name self, name code, datastream<const char *> ds);

    ACTION setprofile(name user, string name_in_chat, string description, string profile_img_url);
    ACTION delprofile(name user);
    ACTION setchat(name adder, name chat_account, string permission, string description, string community_profile_img_url, string community_background_img_url, string community_name);
    ACTION delchat(name chat_account);
    ACTION delmessage(name user, uint64_t message_id, name chat_account);
    ACTION delmessages(name chat_account, uint16_t number_of_messages);
    ACTION deloffon(name chat_account, bool delon);
    ACTION cleardata();
    ACTION migratemes();
    ACTION migratecom();
    ACTION signin(string memo);
    ACTION pinmessage(name user, name community, uint64_t message_id);
    ACTION saveproposal(name proposer, name community, name proposal_name, const std::vector<char>& packed_transaction, const string& title, const string& description, const name& creator);
    ACTION rememoji(name user, uint64_t message_id, uint16_t emoji_id, name chat_account);
    ACTION addemoji(name user, uint64_t message_id, uint16_t emoji_id, name chat_account);
    ACTION sendmsg(string message, name user, name chat_account,string replied_to);
    ACTION unpinmessage(name user, name community, uint64_t message_id);
    ACTION execprop(name community, name proposal_name);
    ACTION delprop(name community, name proposal_name);




private:

void require_auth_either(name user1, name user2);


};
