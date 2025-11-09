#pragma once

#include <concepts>
#include <coroutine>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "mafia/logger.h"
#include "mafia/roles.h"
#include "mafia/shared_ptr.h"

template <typename T>
T choose_random(const std::vector<T>& candidate_options) {
    std::random_device random_device;
    std::mt19937 random_generator(random_device());
    std::uniform_int_distribution<> distribution(0, static_cast<int>(candidate_options.size()) - 1);

    return candidate_options[distribution(random_generator)];
}

struct Action {
    struct promise_type {
        Action get_return_object();
        std::suspend_always initial_suspend();
        std::suspend_always final_suspend() noexcept;
        void return_void();
        void unhandled_exception();
    };

    std::coroutine_handle<promise_type> coroutine_handle;

    ~Action();
};

class Player {
public:
    int selected_target_id = -1;
    int kill_target_id = -1;
    int previous_target_id = -1;

    virtual ~Player() = default;
    virtual Role role() const = 0;

    virtual Class player_class() const {
        return get_faction(role());
    }

    virtual Action act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                       int player_id,
                       bool is_mafia_round) = 0;

    virtual Action vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                        int player_id) = 0;

    virtual std::string role_label() const {
        return role_name(role());
    }

    virtual int get_target() const {
        return selected_target_id;
    }

    virtual void set_target(int target_id) {
        selected_target_id = target_id;
    }
};

template <typename T>
concept ValidRole = requires(T role_object, int player_id, const std::map<int, SharedPtr<Player>>& alive_players_map) {
    { role_object.act(alive_players_map, player_id, true) } -> std::same_as<Action>;
    { role_object.vote(alive_players_map, player_id) } -> std::same_as<Action>;
    { role_object.role_label() } -> std::convertible_to<std::string>;
    { role_object.set_target(0) };
    { role_object.get_target() } -> std::convertible_to<int>;
};
