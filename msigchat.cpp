#include "msigchat.hpp"

msigchat::msigchat(
    name self,
    name code,
    datastream<const char *> ds
) : contract(self, code, ds)
{}

void msigchat::setprofile(name user, string name_in_chat, string description, string profile_img_url) 
{
    require_auth(user);

    check(name_in_chat.size() > 0 && name_in_chat.size() < 64, "name_in_chat length not appropriate");
    check(description.size() < 256, "Description too long");
    check(profile_img_url.size() < 512, "Image URL too long");

    profile_table profiles(get_self(), get_self().value);
    auto itr = profiles.find(user.value);

    if(itr == profiles.end())
    {
        profiles.emplace(_self, [&](auto& row) {
            row.user = user;
            row.name_in_chat = name_in_chat;
            row.description = description;
            row.profile_img_url = profile_img_url;
        });
    } 
    else 
    {
        profiles.modify(itr, user, [&](auto& row) {
            row.name_in_chat = name_in_chat;
            row.description = description;
            row.profile_img_url = profile_img_url;
        });
    }
}

void msigchat::delprofile(name user) {
    require_auth(user);

    profile_table profiles(get_self(), get_self().value);
    auto itr = profiles.find(user.value);
    check(itr != profiles.end(), "Profile not found");
    profiles.erase(itr);
}

void msigchat::setchat(name adder, name chat_account, string permission, string description, string community_profile_img_url, string community_background_img_url) {
    
    //require auth adder or self.

    chats_table chats(get_self(), get_self().value);
    auto itr = chats.find(chat_account.value);

    if(itr == chats.end()) 
    {
        require_auth(adder);

        chats.emplace(_self, [&](auto& row) {
            row.chat_account = chat_account;
            row.permission = permission;
            row.description = description;
            row.community_profile_img_url = community_profile_img_url;
            row.community_background_img_url = community_background_img_url;
        });
    } 
    else 
    {

        require_auth(_self);

        chats.modify(itr, _self, [&](auto& row) {
            row.permission = permission;
            row.description = description;
            row.community_profile_img_url = community_profile_img_url;
            row.community_background_img_url = community_background_img_url;
        });
    }
}

void msigchat::delchat(name chat_account) 
{
    require_auth(get_self());

    delapproval_table approvals(get_self(), get_self().value);
    auto approval_itr = approvals.find(chat_account.value);
    check(approval_itr != approvals.end(), "Chat account not found in approvals");
    check(approval_itr->approved_to_delete, "Deletion not approved for this chat");

    chats_table chats(get_self(), get_self().value);
    auto chat_itr = chats.find(chat_account.value);
    check(chat_itr != chats.end(), "Chat account not found");
    chats.erase(chat_itr);
}

void msigchat::sendmessage(string message, name user, name chat_account) 
{
    require_auth(_self);
    
    check(message.size() > 0 && message.size() < 1000, "Invalid message length");

    messages_table messages(get_self(), chat_account.value);
    messages.emplace(_self, [&](auto& row) {
        row.id = messages.available_primary_key();
        row.message = message;
        row.user = user;
    });
}

void msigchat::sendmsg(string message, name user, name chat_account) 
{
    require_auth(_self);
    
    check(message.size() > 0 && message.size() < 1000, "Invalid message length");

    messages_tb messages(get_self(), chat_account.value);
    messages.emplace(_self, [&](auto& row) {
        row.id = messages.available_primary_key();
        row.message = message;
        row.user = user;
        row.message_time = current_time_point();
    });
}

void msigchat::cleardata() {
    require_auth(_self); // Only the contract itself can run this action

    // List of known communities
    vector<name> communities = {
        "allcustodian"_n, "eyeke.dac"_n, "ipfmultisig2"_n, "ipfmultisig3"_n,
        "kavian.dac"_n, "magor.dac"_n, "msigchatdemo"_n, "naron.dac"_n,
        "neri.dac"_n, "veles.dac"_n
    };

    // Loop through each community and clear data
    for (const auto& community : communities) {
        messages_tb messages(_self, community.value);

        // Delete each entry in the table
        auto itr = messages.begin();
        while (itr != messages.end()) {
            itr = messages.erase(itr);
        }
    }
}

void msigchat::migrate() {
    require_auth(_self); // Only the contract itself can run this migration

    // List of known communities
    vector<name> communities = {
        "allcustodian"_n, "eyeke.dac"_n, "ipfmultisig2"_n, "ipfmultisig3"_n,
        "kavian.dac"_n, "magor.dac"_n, "msigchatdemo"_n, "naron.dac"_n,
        "neri.dac"_n, "veles.dac"_n
    };

 // Loop through each community
    for (const auto& community : communities) {
        // Access the old and new tables scoped by the community
        messages_table old_messages(_self, community.value);
        messages_tb new_messages(_self, community.value);

        // Copy all old message IDs to a vector
        vector<uint64_t> ids;
        for (auto itr = old_messages.begin(); itr != old_messages.end(); itr++) {
            ids.push_back(itr->id);
        }

        // Iterate over the vector in reverse
        for (auto itr = ids.rbegin(); itr != ids.rend(); itr++) {
            auto old_itr = old_messages.find(*itr);
            if (old_itr != old_messages.end()) {
                // Insert this entry into the new table
                new_messages.emplace(_self, [&](auto& msg) {
                    msg.id = old_itr->id;
                    msg.message = old_itr->message;
                    msg.user = old_itr->user;
                    // msg.message_time will be default-initialized to epoch time 0
                });
            }
        }
    }
}

void msigchat::delmessage(name user, uint64_t message_id, name chat_account) 
{
    require_auth_either(user, _self);

    messages_tb messages(get_self(), chat_account.value);
    auto msg_itr = messages.find(message_id);
    check(msg_itr != messages.end(), "Message not found");
    check(msg_itr->user == user, "Only the author can delete this message");

    messages.erase(msg_itr);
}

void msigchat::delmessages(name chat_account, uint16_t number_of_messages) 
{
    require_auth_either(chat_account, _self);

    delapproval_table approvals(get_self(), get_self().value);
    auto approval_itr = approvals.find(chat_account.value);
    check(approval_itr != approvals.end(), "Chat account not found in approvals");
    check(approval_itr->approved_to_delete, "Deletion not approved for this chat");

    messages_tb messages(get_self(), chat_account.value);

    for (int i = 0; i < number_of_messages; i++) 
    {
        auto msg_itr = messages.begin();
        if (msg_itr == messages.end()) {
            break;
        }
        messages.erase(msg_itr);
    }
}

void msigchat::deloffon(name chat_account, bool delon) 
{
    require_auth_either(chat_account, _self); 

    delapproval_table approvals(get_self(), get_self().value);
    auto approval_itr = approvals.find(chat_account.value);

    if (approval_itr == approvals.end()) {
        approvals.emplace(get_self(), [&](auto& row) {
            row.chat_account = chat_account;
            row.approved_to_delete = delon;
        });
    } else {
        approvals.modify(approval_itr, get_self(), [&](auto& row) {
            row.approved_to_delete = delon;
        });
    }
}

void msigchat::signin(string memo) 
{

}

void msigchat::require_auth_either(name user1, name user2) {
  if (has_auth(user1)) {
    require_auth(user1);
  } else if (has_auth(user2)) {
    require_auth(user2);
  } else {
    check(false, "Missing required authority");
  }
}