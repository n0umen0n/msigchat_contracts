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
        profiles.modify(itr, _self, [&](auto& row) {
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

void msigchat::setchat(name adder, name chat_account, string permission, string description, string community_profile_img_url, string community_background_img_url, string community_name) {
    
    //require auth adder or self.

    chats1_table chats(get_self(), get_self().value);
    auto itr = chats.find(chat_account.value);

    if(itr == chats.end()) 
    {
        //Is it adder or _self
        require_auth(adder);

        chats.emplace(_self, [&](auto& row) {
            row.chat_account = chat_account;
            row.permission = permission;
            row.description = description;
            row.community_profile_img_url = community_profile_img_url;
            row.community_background_img_url = community_background_img_url;
            row.community_name = community_name;
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
            row.community_name = community_name;

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

    chats1_table chats(get_self(), get_self().value);
    auto chat_itr = chats.find(chat_account.value);
    check(chat_itr != chats.end(), "Chat account not found");
    chats.erase(chat_itr);
}
/*
void msigchat::sendmessage(string message, name user, name chat_account) 
{
    require_auth(_self);
    
    check(message.size() > 0 && message.size() < 1000, "Invalid message length");

    messages_tb messages(get_self(), chat_account.value);
    messages.emplace(_self, [&](auto& row) {
        row.id = messages.available_primary_key();
        row.message = message;
        row.user = user;
    });
}
*/

void msigchat::pinmessage(name user, name community, uint64_t message_id) 
    {
        require_auth(_self); // Ensure that the action is executed by the user

        // Access the messages table
        messages2_tb messages(get_self(), community.value);
        auto msg_itr = messages.find(message_id);
        check(msg_itr != messages.end(), "Message not found"); // Ensure the message exists

        // Access the pinmessage table
        pinmessage_tb pinmessages(get_self(), community.value);
        auto pin_itr = pinmessages.find(message_id);

        // Current time as the pin time
        auto current_time = current_time_point();

        // If the pinned message already exists for the community, replace it
        if (pin_itr != pinmessages.end()) {
            pinmessages.modify(pin_itr, _self, [&](auto& pin) {
                pin.pinned_message = msg_itr->message;
                pin.user_who_pinned = user;
                pin.user_who_created_message = msg_itr->user;
                pin.message_time = msg_itr->message_time;
                pin.pin_time = current_time;
                pin.emojis = msg_itr->emojis;
            });
        } else {
            // Otherwise, create a new pinned message entry
            pinmessages.emplace(_self, [&](auto& pin) {
                pin.community = community;
                pin.pinned_message = msg_itr->message;
                pin.user_who_pinned = user;
                pin.user_who_created_message = msg_itr->user;
                pin.message_time = msg_itr->message_time;
                pin.pin_time = current_time;
                pin.emojis = msg_itr->emojis;
                pin.message_id = message_id;

            });
        }
    }

void msigchat::unpinmessage(name user, name community, uint64_t message_id) {
    require_auth(_self); // Ensure that the action can only be executed by the contract itself or an authorized user

    // Access the pinmessage table
    pinmessage_tb pinmessages(get_self(), community.value);
    auto pin_itr = pinmessages.find(message_id);

    // Check if there's a pinned message for the community
    check(pin_itr != pinmessages.end(), "No pinned message found for the community");

    // Erase the pinned message entry
    pinmessages.erase(pin_itr);
}




void msigchat::sendmsg(string message, name user, name chat_account, string replied_to) {
    require_auth(_self); // Ensure that only the contract account can execute this action
    
    check(message.size() > 0 && message.size() < 5000, "Invalid message length"); // Validate message length

    messages2_tb messages(get_self(), chat_account.value); // Access the messages table scoped by chat_account
    messages.emplace(_self, [&](auto& row) {
        row.id = messages.available_primary_key(); // Automatically generate a unique ID for the message
        row.message = message; // Set the message content
        row.user = user; // Set the user who sent the message
        row.message_time = current_time_point(); // Record the current time as the message time
        row.replied_to = replied_to;
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
        messages2_tb messages(_self, community.value);

        // Delete each entry in the table
        auto itr = messages.begin();
        while (itr != messages.end()) {
            itr = messages.erase(itr);
        }
    }
}

void msigchat::delmessage(name user, uint64_t message_id, name chat_account) 
{
    require_auth_either(user, _self);

    messages2_tb messages(get_self(), chat_account.value);
    auto msg_itr = messages.find(message_id);
    check(msg_itr != messages.end(), "Message not found");
    check(msg_itr->user == user, "Only the author can delete this message");

    messages.erase(msg_itr);
}


void msigchat::addemoji(name user, uint64_t message_id, uint16_t emoji_id, name chat_account) {
    require_auth(_self);

    messages2_tb messages(get_self(), chat_account.value);
    auto msg_itr = messages.find(message_id);
    check(msg_itr != messages.end(), "Message not found");

    // Modify the message to update emoji reactions
    messages.modify(msg_itr, same_payer, [&](auto& msg) {
        auto emoji_itr = std::find_if(msg.emojis.begin(), msg.emojis.end(), [&](const auto& emoji) {
            return emoji.emoji_id == emoji_id;
        });

        if (emoji_itr != msg.emojis.end()) {
            // If the emoji is already present, check if the user has already reacted
            if (std::find(emoji_itr->users.begin(), emoji_itr->users.end(), user) == emoji_itr->users.end()) {
                emoji_itr->count += 1; // Increment the count only if the user hasn't reacted before
                emoji_itr->users.push_back(user); // Add the user to the list of users who reacted
            }
        } else {
            // If the emoji wasn't found, add it with the user who reacted
            msg.emojis.push_back(emoji_reaction{emoji_id, 1, {user}});
        }
    });
}

void msigchat::saveproposal(name proposer, name community, name proposal_name, const std::vector<char>& packed_transaction, const string& title, const string& description) {
    require_auth(_self); // Ensure the proposer is authorized to create or update a proposal

    proposals_tb proposals(get_self(), community.value); // Access the proposals table scoped by community
    auto prop_itr = proposals.find(proposal_name.value); // Find the proposal by name

    if (prop_itr == proposals.end()) {
        // If the proposal does not exist, create a new entry
        proposals.emplace(proposer, [&](auto& p) {
            p.proposal_name = proposal_name;
            p.packed_transaction = packed_transaction;
            p.title = title;
            p.description = description;
            p.community = community;
            p.proposal_time = current_time_point();
            p.proposer = proposer;

        });
    } else {
        // If the proposal already exists, update its details
        proposals.modify(prop_itr, same_payer, [&](auto& p) {
            p.title = title;
            p.description = description;
        });
    }
}



void msigchat::rememoji(name user, uint64_t message_id, uint16_t emoji_id, name chat_account) {
    require_auth(_self); // Ensure that the action is executed by the user

    messages2_tb messages(get_self(), chat_account.value); // Access the messages table scoped by chat_account
    auto msg_itr = messages.find(message_id); // Find the message by message_id
    check(msg_itr != messages.end(), "Message not found"); // Ensure the message exists

    // Flag to check if the emoji was found and removed
    bool found_and_removed = false;

    // Modify the message to update emoji reactions
    messages.modify(msg_itr, same_payer, [&](auto& msg) {
        // Iterate over emojis in the message
        auto emoji_itr = msg.emojis.begin();
        while (emoji_itr != msg.emojis.end()) {
            if (emoji_itr->emoji_id == emoji_id) { // Check if this is the emoji to remove
                // Find the user in the list of users who reacted with this emoji
                auto user_itr = std::find(emoji_itr->users.begin(), emoji_itr->users.end(), user);
                if (user_itr != emoji_itr->users.end()) { // User found
                    emoji_itr->users.erase(user_itr); // Remove the user from the list of users
                    emoji_itr->count -= 1; // Decrement the count of reactions
                    found_and_removed = true;

                    // If no users left who reacted with this emoji, remove the emoji reaction
                    if (emoji_itr->users.empty()) {
                        emoji_itr = msg.emojis.erase(emoji_itr); // Erase the emoji reaction and move to the next
                        continue; // Continue to the next iteration of the loop, skipping the increment
                    }
                }
            }
            ++emoji_itr; // Manually increment the iterator only if not erased
        }
    });

    check(found_and_removed, "Emoji reaction not found for user"); // Ensure the emoji reaction was found and removed
}

void msigchat::delmessages(name chat_account, uint16_t number_of_messages) 
{
    require_auth_either(chat_account, _self);

    delapproval_table approvals(get_self(), get_self().value);
    auto approval_itr = approvals.find(chat_account.value);
    check(approval_itr != approvals.end(), "Chat account not found in approvals");
    check(approval_itr->approved_to_delete, "Deletion not approved for this chat");

    messages2_tb messages(get_self(), chat_account.value);

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

void msigchat::migrate_communities() {
    require_auth(_self); // Ensure only the contract can execute this

    chats_table old_chats(_self, _self.value);
    chats1_table new_chats(_self, _self.value);

    for (auto itr = old_chats.begin(); itr != old_chats.end(); itr++) {
        // Check if the entry already exists in the new table and skip or update
        auto new_itr = new_chats.find(itr->chat_account.value);
        if (new_itr == new_chats.end()) {
            // Convert chat_account to string for community_name
            new_chats.emplace(_self, [&](auto& row) {
                row.chat_account = itr->chat_account;
                row.permission = itr->permission;
                row.description = itr->description;
                row.community_profile_img_url = itr->community_profile_img_url;
                row.community_background_img_url = itr->community_background_img_url;
                row.community_name = itr->chat_account.to_string();
            });
        }
    }
}

void msigchat::migrate_messages() {
    require_auth(_self); // Ensure only the contract can execute this

    chats_table communities(_self, _self.value); // Access the chats table to find communities

    // Iterate over each community in the chats table
    for (auto& comm : communities) {
        name community = comm.chat_account; // chat_account is the community name
        messages_tb old_messages(_self, community.value);
        messages2_tb new_messages(_self, community.value);

        // Since EOSIO doesn't support reverse iteration directly, first collect all message IDs
        vector<uint64_t> ids;
        for (auto itr = old_messages.begin(); itr != old_messages.end(); itr++) {
            ids.push_back(itr->id);
        }

        // Then iterate over collected IDs in reverse to maintain the original message order
        for (auto itr = ids.rbegin(); itr != ids.rend(); itr++) {
            auto old_itr = old_messages.find(*itr);
            if (old_itr != old_messages.end()) {
                // Migrate each message to the new table
                new_messages.emplace(_self, [&](auto& msg) {
                    msg.id = old_itr->id;
                    msg.message = old_itr->message;
                    msg.user = old_itr->user;
                    msg.message_time = old_itr->message_time;
                    // Emojis and replied_to are initialized as empty
                    msg.emojis = vector<emoji_reaction>(); // Assuming an appropriate constructor exists
                    msg.replied_to = ""; // Empty string for non-replied messages
                });
            }
        }
    }
}


/*
void msigchat::addemoji(name user, uint64_t message_id, uint16_t emoji_id, name chat_account) {
    require_auth(_self);

    messages2_tb messages(get_self(), chat_account.value);
    auto msg_itr = messages.find(message_id);
    check(msg_itr != messages.end(), "Message not found");

    // Check if the emoji already exists for the message
    bool emoji_found = false;
    messages.modify(msg_itr, user, [&](auto& msg) {
        for (auto& emoji : msg.emojis) {
            if (emoji.emoji_id == emoji_id) {
                emoji.count += 1; // Increment the count
                emoji_found = true;
                break;
            }
        }
        // If the emoji wasn't found, add it to the message's emojis vector
        if (!emoji_found) {
            msg.emojis.push_back(emoji_count{emoji_id, 1});
        }
    });
}
*/
/*
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
        messages_tb old_messages(_self, community.value);
        messages2_tb new_messages(_self, community.value);

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
*/