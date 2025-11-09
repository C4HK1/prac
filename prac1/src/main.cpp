#include <algorithm>
#include <concepts>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
#include <ranges>
#include <map>
#include <vector>

#include "mafia/core.h"
#include "mafia/roles/civilian.h"
#include "mafia/roles/mafia.h"
#include "mafia/roles/commissioner.h"
#include "mafia/roles/doctor.h"
#include "mafia/roles/maniac.h"
#include "mafia/roles/journalist.h"
#include "mafia/roles/kamikadze.h"
#include "mafia/roles/hacker.h"

static_assert(ValidRole<Civilian>);
static_assert(ValidRole<Mafia>);
static_assert(ValidRole<Journalist>);
static_assert(ValidRole<Commissioner>);
static_assert(ValidRole<Doctor>);
static_assert(ValidRole<Maniac>);
static_assert(ValidRole<Hacker>);
static_assert(ValidRole<Kamikadze>);

int user(std::map<int, SharedPtr<Player>>& alive_players, std::mt19937& gen) {
    std::vector<int> player_ids;

    for (const auto& [id, player] : alive_players) {
        if (player->role() != Role::Civilian) {
            player_ids.push_back(id);
        }
    }

    std::shuffle(player_ids.begin(), player_ids.end(), gen);
    int user_id = player_ids[0];
    std::cout << "Вы играете за игрока с ID: " << user_id << ". Ваша роль: " << alive_players[user_id]->role_label() << std::endl;

    if (alive_players[user_id]->player_class() == Class::Mafia) {
        std::cout << "Ваша команда мафии:" << std::endl;
        for (const auto& [id, player] : alive_players) {
            if (player->player_class() == Class::Mafia && id != user_id) {
                std::cout << "Игрок " << id << " (" << player->role_label() << ")" << std::endl;
            }
        }
    }
    return user_id;
}

void night(std::map<int, SharedPtr<Player>>& alive_players, int user_id) {
    if (alive_players.find(user_id) == alive_players.end()) {
        std::cout << "Вы не можете выполнить действие, так как ваш персонаж больше не жив." << std::endl;
        return;
    } else {
        std::cout << "Игрок жив. " << "Ночной раунд : " << Logger::get_current_round() << std::endl;
        switch (alive_players[user_id].get()->role()) {
            case Role::Civilian:
                std::cout << "Вы мирный, пропуск ночной фазы." << std::endl;
                return;
            case Role::Commissioner: {
                std::cout << "1 - Проверить игрока." << std::endl;
                std::cout << "2 - Выстрелить в игрока." << std::endl;
                int mode;
                std::cin >> mode;
                while (mode != 1 && mode != 2) {
                    std::cout << "1 - Проверить игрока." << std::endl;
                    std::cout << "2 - Выстрелить в игрока." << std::endl;
                    std::cin >> mode;
                }
                for (const auto& [id, _] : alive_players)
                    if (id != user_id)
                        std::cout << "Игрок " << id << std::endl;
                int target = -1;
                if (mode == 1) {
                    std::cout << "Выберите ID игрока для проверки: " << std::endl;
                    std::cin >> target;
                    while (alive_players.find(target) == alive_players.end() || target == user_id) {
                        std::cout << "Неверный ID игрока." << std::endl;
                        std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                        std::cin >> target;
                    }
                    if (alive_players[target].get()->player_class() == Class::Mafia) {
                        std::cout << "Выбранный игрок мафия." << std::endl;
                    } else {
                        std::cout << "Выбранный игрок не мафия." << std::endl;
                    }
                } else if (mode == 2) {
                    std::cout << "Выберите ID игрока для убийства: " << std::endl;
                    std::cin >> target;
                    while (alive_players.find(target) == alive_players.end() || target == user_id) {
                        std::cout << "Неверный ID игрока." << std::endl;
                        std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                        std::cin >> target;
                    }
                    dynamic_cast<Commissioner*>(alive_players[user_id].get())->set_kill(target);
                }
                Logger::get_instance().log_round("Игрок " + std::to_string(user_id) + " выбрал целью игрока " + std::to_string(target));
                return;
            }
            case Role::Doctor: {
                for (const auto& [id, _] : alive_players)
                    if (id != user_id)
                        std::cout << "Игрок " << id << std::endl;
                std::cout << "Выберите ID игрока для исцеления: " << std::endl;
                int target;
                std::cin >> target;
                while (alive_players.find(target) == alive_players.end() || target == dynamic_cast<Doctor*>(alive_players[user_id].get())->get_prev_target()) {
                    std::cout << "Неверный ID игрока. ID не должен быть таким же как в прошлом раунде и должен принадлежать списку." << std::endl;
                    std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                    std::cin >> target;
                }
                dynamic_cast<Doctor*>(alive_players[user_id].get())->set_prev_target(target);
                alive_players[user_id].get()->set_target(target);
                Logger::get_instance().log_round("Игрок " + std::to_string(user_id) + " выбрал целью игрока " + std::to_string(target));
                return;
            }
            case Role::Journalist: {
                for (const auto& [id, _] : alive_players)
                    if (id != user_id)
                        std::cout << "Игрок " << id << std::endl;
                std::cout << "Выберите первый ID игрока для интервью: " << std::endl;
                int target, target2;
                std::cin >> target;
                while (alive_players.find(target) == alive_players.end()) {
                    std::cout << "ID игрока должен принадлежать списку." << std::endl;
                    std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                    std::cin >> target;
                }
                std::cin >> target2;
                while (alive_players.find(target) == alive_players.end() || target == target2) {
                    std::cout << "Неверный ID игрока. ID не должен быть таким же как в прошлом раунде и должен принадлежать списку." << std::endl;
                    std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                    std::cin >> target;
                }
                if (alive_players[target].get()->player_class() != alive_players[target2].get()->player_class()) {
                    std::cout << "Данные игроки - ВРАГИ!" << std::endl;
                } else {
                    std::cout << "Данные игроки - друзья!" << std::endl;
                }
                Logger::get_instance().log_round("Игрок " + std::to_string(user_id) + " выбрал целью игроков " + std::to_string(target) + " и " + std::to_string(target2));
                return;
            }
            case Role::Maniac: {
                for (const auto& [id, _] : alive_players)
                    if (id != user_id)
                        std::cout << "Игрок " << id << std::endl;
                std::cout << "Выберите ID игрока для убийства: " << std::endl;
                int target;
                std::cin >> target;
                while (alive_players.find(target) == alive_players.end() || target == user_id) {
                    std::cout << "ID игрока должен принадлежать списку." << std::endl;
                    std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                    std::cin >> target;
                }
                alive_players[user_id].get()->set_target(target);
                Logger::get_instance().log_round("Игрок " + std::to_string(user_id) + " выбрал целью игрока " + std::to_string(target));
                return;
            }
            case Role::Mafia:
            case Role::Don:
            case Role::Kamikadze: {
                for (const auto& [id, player] : alive_players) {
                    if (alive_players[id].get()->player_class() != Class::Mafia) {
                        std::cout << "Игрок " << id << std::endl;
                    } else {
                        std::cout << "Игрок " << id << " (" << player->role_label() << ")" << std::endl;
                    }
                }
                std::cout << "Выберите ID игрока для убийства: " << std::endl;
                int target;
                std::cin >> target;
                while (alive_players.find(target) == alive_players.end() || alive_players[target].get()->player_class() == Class::Mafia) {
                    std::cout << "ID игрока должен принадлежать списку." << std::endl;
                    std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                    std::cin >> target;
                }
                alive_players[user_id].get()->set_target(target);
                Logger::get_instance().log_round("Игрок " + std::to_string(user_id) + " выбрал целью игрока " + std::to_string(target));
                if (alive_players[user_id].get()->role() == Role::Kamikadze) {
                    if (alive_players[target].get()->role() == Role::Commissioner) {
                        std::cout << "Комиссар попался!" << std::endl;
                    } else {
                        std::cout << "Это не комиссар!" << std::endl;
                    }
                }
                return;
            }
            case Role::Hacker: {
                for (const auto& [id, _] : alive_players)
                    if (id != user_id)
                        std::cout << "Игрок " << id << std::endl;
                std::cout << "Выберите ID игрока для взлома: " << std::endl;
                int target;
                std::cin >> target;
                while (alive_players.find(target) == alive_players.end() || target == user_id) {
                    std::cout << "ID игрока должен принадлежать списку." << std::endl;
                    std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                    std::cin >> target;
                }
                alive_players[user_id].get()->set_target(target);
                std::cout << "Вы взломали игрока " << target << ". Он оказался " << alive_players[user_id].get()->role_label() << std::endl;
                Logger::get_instance().log_round("Игрок " + std::to_string(user_id) + " выбрал целью игрока " + std::to_string(target));
                return;
            }
            default:
                return;
        }
    }
}

void vote(std::map<int, SharedPtr<Player>>& alive_players, int user_id) {
    if (alive_players.find(user_id) == alive_players.end()) {
        std::cout << "Вы не можете голосовать, так как ваш персонаж больше не жив." << std::endl;
        return;
    }
    std::cout << "Игрок жив. " << "Дневной раунд : " << Logger::get_current_round() << std::endl;
    std::cout << "Ваш голос! Выберите игрока для голосования (введите ID):" << std::endl;
    bool is_maf = alive_players[user_id].get()->player_class() == Class::Mafia;
    for (const auto& [id, player] : alive_players) {
        if (id != user_id) {
            if (is_maf && player->player_class() == Class::Mafia) {
                std::cout << "Игрок " << id << " (" << player->role_label() << ")" << std::endl;
            } else {
                std::cout << "Игрок " << id << std::endl;
            }
        }
    }
    int vote_target;
    std::cin >> vote_target;
    while (vote_target == user_id || alive_players.find(vote_target) == alive_players.end()) {
        std::cout << "Неверный ID игрока. ID должен принадлежать списку." << std::endl;
        std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
        std::cin >> vote_target;
    }
    alive_players[user_id].get()->set_target(vote_target);
    Logger::get_instance().log_round("Игрок " + std::to_string(user_id) + " голосует за игрока " + std::to_string(vote_target));
}

void night(std::map<int, SharedPtr<Player>>& alive_players, bool user_in_game, int user_id) {
    std::map<int, int> vote_count;
    std::vector<std::future<void>> futures;
    int victim_mafia = -1;
    int victim_maniac = -1;
    int victim_com = -1;
    int don_id = -1;
    int victim_doctor = -1;
    int kamikadze_id = -1;

    for (const auto& [id, player] : alive_players) {
        if (player->role() == Role::Don) {
            don_id = id;
        } else if (player->role() == Role::Kamikadze) {
            kamikadze_id = id;
        } else if (kamikadze_id != -1 && don_id != -1) {
            break;
        }
    }

    for (auto& [id, player] : alive_players) {
        Role r = player->role();
        if (user_in_game && id == user_id) {
            if (r != Role::Civilian) {
                night(alive_players, user_id);
            }
        } else {
            if (r != Role::Civilian) {
                futures.push_back(std::async(std::launch::async, [&alive_players, id]() {
                    Action action = alive_players[id].get()->act(alive_players, id, true);
                    action.coroutine_handle.resume();
                }));
            }
        }
    }

    for (auto& future : futures) {
        future.get();
    }
    futures.clear();

    if (don_id != -1 && user_id != don_id) {
        futures.push_back(std::async(std::launch::async, [&alive_players, don_id]() {
            Action action = alive_players.at(don_id).get()->act(alive_players, don_id, false);
            action.coroutine_handle.resume();
        }));
    }

    for (auto& future : futures) {
        future.get();
    }

    if (don_id != -1) {
        Mafia* don = dynamic_cast<Mafia*>(alive_players[don_id].get());
        victim_mafia = don->selected_target_id;
    }

    auto maniac_player = alive_players | std::views::filter([](const auto& e) { return e.second->role() == Role::Maniac; });
    auto commissioner_player = alive_players | std::views::filter([](const auto& e) { return e.second->role() == Role::Commissioner; });
    auto doctor_player = alive_players | std::views::filter([](const auto& e) { return e.second->role() == Role::Doctor; });
    auto hacker_player = alive_players | std::views::filter([](const auto& e) { return e.second->role() == Role::Hacker; });

    if (!std::ranges::empty(hacker_player)) {
        int target = hacker_player.begin()->second.get()->get_target();
        auto k = alive_players.find(target);
        if (k != alive_players.end()) {
            if (user_in_game) {
                std::cout << "Хакер взломал игрока " << target << ". Он оказался " << k->second.get()->role_label() << std::endl;
            }
        }
    }

    Kamikadze* kamikadze_player = NULL;
    if (kamikadze_id != -1)
        kamikadze_player = dynamic_cast<Kamikadze*>(alive_players[kamikadze_id].get());

    if (!std::ranges::empty(doctor_player))
        victim_doctor = dynamic_cast<Doctor*>(doctor_player.begin()->second.get())->selected_target_id;
    if (!std::ranges::empty(maniac_player))
        victim_maniac = dynamic_cast<Maniac*>(maniac_player.begin()->second.get())->selected_target_id;
    if (!std::ranges::empty(commissioner_player))
        victim_com = dynamic_cast<Commissioner*>(commissioner_player.begin()->second.get())->kill_target_id;

    if (victim_doctor != -1)
        Logger::get_instance().log_round("Доктор лечит игрока " + std::to_string(victim_doctor));
    if (victim_com != -1)
        Logger::get_instance().log_round("Коммиссар стреляет в игрока " + std::to_string(victim_com));
    if (victim_maniac != -1)
        Logger::get_instance().log_round("Маньяк атакует игрока " + std::to_string(victim_maniac));
    if (victim_mafia != -1)
        Logger::get_instance().log_round("Мафия посещает дом игрока " + std::to_string(victim_mafia));

    if (kamikadze_id != -1) {
        if (kamikadze_player->found_commissioner) {
            Logger::get_instance().log_round("Комиссар попался! 3... 2... 1...");
            int poor_com = commissioner_player.begin()->first;
            alive_players.erase(poor_com);
            alive_players.erase(kamikadze_id);
            Logger::get_instance().log_round("Бот " + std::to_string(poor_com) + " был взорван Камикадзе " + std::to_string(kamikadze_id));
            if (user_in_game == true) {
                std::cout << "Бот " + std::to_string(victim_mafia) + " был взорван Камикадзе " + std::to_string(kamikadze_id) << std::endl;
            }
        } else {
            Logger::get_instance().log_round("Камикадзе проверил игрока " + std::to_string(kamikadze_player->get_target()));
        }
    }

    if (victim_mafia != -1 && victim_mafia != victim_doctor) {
        alive_players.erase(victim_mafia);
        Logger::get_instance().log_round("Игрок/Бот " + std::to_string(victim_mafia) + " был убит мафией.");
        if (user_in_game == true) {
            std::cout << "Игрок/Бот " + std::to_string(victim_mafia) + " был убит мафией." << std::endl;
        }
    }

    if (victim_maniac != -1 && victim_maniac != victim_doctor) {
        alive_players.erase(victim_maniac);
        Logger::get_instance().log_round("Игрок/Бот " + std::to_string(victim_maniac) + " был убит маньяком.");
        if (user_in_game == true) {
            std::cout << "Игрок/Бот " + std::to_string(victim_maniac) + " был убит маньяком." << std::endl;
        }
    }

    if (victim_com != -1 && victim_com != victim_doctor) {
        alive_players.erase(victim_com);
        Logger::get_instance().log_round("Игрок/Бот " + std::to_string(victim_com) + " был убит комиссаром.");
        if (user_in_game == true) {
            std::cout << "Игрок/Бот " + std::to_string(victim_com) + " был убит комиссаром." << std::endl;
        }
    }

    don_id = -1;
    std::vector<int> mafia_ids = {};
    kamikadze_id = -1;
    for (const auto& [id, player] : alive_players) {
        if (player->role() == Role::Don) {
            don_id = id;
            break;
        } else if (player->role() == Role::Mafia) {
            mafia_ids.push_back(id);
        } else if (player->role() == Role::Kamikadze) {
            mafia_ids.push_back(id);
            kamikadze_id = id;
        }
    }

    if (don_id == -1 && !mafia_ids.empty()) {
        if (kamikadze_id != -1 && mafia_ids.size() == 1) {
            auto don = new Mafia();
            don->is_don_leader = true;
            alive_players[kamikadze_id] = SharedPtr<Player>(don);
        } else {
            if (kamikadze_id != -1)
                mafia_ids.erase(std::find(mafia_ids.begin(), mafia_ids.end(), kamikadze_id));
            don_id = choose_random(mafia_ids);
            dynamic_cast<Mafia*>(alive_players[don_id].get())->is_don_leader = true;
            Logger::get_instance().log_round("Игрок " + std::to_string(don_id) + " назначен новым Доном мафии.");
        }
    }

    if (user_in_game)
        std::cout << "\n\nНочь завершена\n\n"
                  << std::endl;
}

void day(std::map<int, SharedPtr<Player>>& alive_players, bool user_in_game, int user_id) {
    std::map<int, int> vote_count;
    std::vector<std::future<void>> futures;

    for (auto& [id, player] : alive_players) {
        futures.push_back(std::async(std::launch::async, [&alive_players, id, user_in_game, user_id]() {
            if (user_in_game && id == user_id)
                vote(alive_players, user_id);
            else {
                Action action = alive_players[id].get()->vote(alive_players, id);
                action.coroutine_handle.resume();
            }
        }));
    }

    for (auto& future : futures)
        future.get();

    for (const auto& [id, player] : alive_players) {
        int t = player->get_target();
        if (t != -1)
            ++vote_count[t];
    }

    Role voted_off_role = Role::Civilian;
    if (!vote_count.empty()) {
        int max_votes = std::max_element(vote_count.begin(), vote_count.end(), [](const auto& a, const auto& b) { return a.second < b.second; })->second;
        std::vector<int> leaders;
        for (const auto& [pid, cnt] : vote_count)
            if (cnt == max_votes)
                leaders.push_back(pid);
        int victim = choose_random(leaders);
        voted_off_role = alive_players[victim].get()->role();
        alive_players.erase(victim);
        Logger::get_instance().log_round("Игрок/Бот " + std::to_string(victim) + " был убит после дневного голосования.");
        if (user_in_game == true)
            std::cout << "Игрок/Бот " + std::to_string(victim) + " был убит после дневного голосования." << std::endl;
    }

    if (voted_off_role == Role::Don) {
        std::vector<int> mafia_ids = {};
        int kamikadze_id = -1;
        for (const auto& [id, player] : alive_players) {
            if (player->role() == Role::Mafia)
                mafia_ids.push_back(id);
            else if (player->role() == Role::Kamikadze) {
                mafia_ids.push_back(id);
                kamikadze_id = id;
            }
        }
        if (!mafia_ids.empty()) {
            int don_id = -1;
            if (kamikadze_id != -1 && mafia_ids.size() == 1) {
                auto don = new Mafia();
                don->is_don_leader = true;
                alive_players[kamikadze_id] = SharedPtr<Player>(don);
                don_id = kamikadze_id;
            } else {
                if (kamikadze_id != -1)
                    mafia_ids.erase(std::find(mafia_ids.begin(), mafia_ids.end(), kamikadze_id));
                don_id = choose_random(mafia_ids);
                dynamic_cast<Mafia*>(alive_players[don_id].get())->is_don_leader = true;
            }
            Logger::get_instance().log_round("Игрок " + std::to_string(don_id) + " назначен новым Доном мафии.");
            if (don_id == user_id)
                std::cout << "Вы стали Доном мафии!!!" << std::endl;
        }
    }

    if (user_in_game)
        std::cout << "\n\nДень завершен\n\n" << std::endl;
}

bool is_end(const std::map<int, SharedPtr<Player>>& alive_players) {
    int mafia_alive = 0, civilians_alive = 0, maniac_alive = 0, neutral_alive = 0;
    for (const auto& [id, player] : alive_players) {
        if (player->player_class() == Class::Mafia)
            ++mafia_alive;
        else if (player->role() == Role::Maniac) {
            ++maniac_alive;
            ++neutral_alive;
        } else if (player->player_class() != Class::Neutral)
            ++civilians_alive;
        else
            ++neutral_alive;
    }

    size_t alive = alive_players.size();
    if (alive == 0) {
        Logger::get_instance().log_final("Ничья!");
        std::cout << "Ничья!" << std::endl;
        return true;
    }
    if (mafia_alive > civilians_alive && maniac_alive == 0) {
        Logger::get_instance().log_final("Мафия победила!");
        std::cout << "Мафия победила!" << std::endl;
        return true;
    }
    if (mafia_alive == 0 && maniac_alive == 0 && civilians_alive > 0) {
        Logger::get_instance().log_final("Мирные жители победили!");
        std::cout << "Мирные жители победили!" << std::endl;
        return true;
    }
    if (maniac_alive > 0 && mafia_alive == 0 && civilians_alive == 0) {
        Logger::get_instance().log_final("Маньяк победил!");
        std::cout << "Маньяк победил!" << std::endl;
        return true;
    }
    if (alive == 1 && neutral_alive > 0) {
        Logger::get_instance().log_final("Хакер победил!");
        std::cout << "Хакер победил!" << std::endl;
        return true;
    }
    return false;
}

void run_game(std::map<int, SharedPtr<Player>>& alive_players, bool user_in_game, int user_id) {
    Logger::reset_round();  // Start from round 0
    while (true) {
        Logger::increment_round();
        Logger::get_instance().log_round("Начало раунда " + std::to_string(Logger::get_current_round()));
        Logger::get_instance().log_round("\nДень\n");
        if (alive_players.size() > 2) {
            for (const auto& [id, player] : alive_players)
                Logger::get_instance().log_round("Игрок " + std::to_string(id) + ": " + player->role_label());
            day(alive_players, user_in_game, user_id);
            if (is_end(alive_players))
                break;
        } else {
            Logger::get_instance().log_round("В городе слишком мало людей для голосования!\n");
            if (user_in_game)
                std::cout << "В городе слишком мало людей для голосования!" << std::endl;
        }
        Logger::get_instance().log_round("\nНочь\n");
        for (const auto& [id, player] : alive_players)
            Logger::get_instance().log_round("Игрок " + std::to_string(id) + ": " + player->role_label());
        night(alive_players, user_in_game, user_id);
        if (is_end(alive_players))
            break;
        // Round counter is now managed by Logger
    }
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    int N;
    std::cout << "Введите количество игроков (N > 5): ";
    std::cin >> N;
    if (N <= 5) {
        std::cout << "Количество игроков должно быть больше 5." << std::endl;
        return 1;
    }
    char user_in_game_choice;
    bool user_in_game = false;
    int user_id = -1;
    std::cout << "Хотите ли вы участвовать в игре? (y/n): ";
    std::cin >> user_in_game_choice;
    if (user_in_game_choice == 'y' || user_in_game_choice == 'Y')
        user_in_game = true;

    int k = 3;
    int mafia_count = std::max(N / k, 1);
    std::map<int, SharedPtr<Player>> alive_players;
    std::vector<SharedPtr<Player>> roles;

    for (int i = 0; i < mafia_count; ++i) {
        if (i == 0) {
            auto don = new Mafia();
            don->is_don_leader = true;
            roles.push_back(SharedPtr<Player>(don));
        } else if (i == 1)
            roles.push_back(SharedPtr<Player>(new Kamikadze()));
        else
            roles.push_back(SharedPtr<Player>(new Mafia()));
    }

    roles.push_back(SharedPtr<Player>(new Commissioner()));
    roles.push_back(SharedPtr<Player>(new Maniac()));
    roles.push_back(SharedPtr<Player>(new Doctor()));
    roles.push_back(SharedPtr<Player>(new Hacker()));

    int civilians_count = N - mafia_count - 4;
    for (int i = 0; i < civilians_count; ++i) {
        if (i == 0)
            roles.push_back(SharedPtr<Player>(new Journalist()));
        else
            roles.push_back(SharedPtr<Player>(new Civilian()));
    }

    std::shuffle(roles.begin(), roles.end(), gen);
    std::vector<int> player_ids;
    for (int i = 1; i <= N; ++i)
        player_ids.push_back(i);
    std::shuffle(player_ids.begin(), player_ids.end(), gen);
    for (int i = 0; i < N; ++i)
        alive_players[player_ids[i]] = roles[i];

    if (user_in_game)
        user_id = user(alive_players, gen);
    for (const auto& [id, player] : alive_players)
        Logger::get_instance().log_round("Игрок " + std::to_string(id) + ": " + player->role_label());
    Logger::get_instance().log_round("Игра началась!");
    run_game(alive_players, user_in_game, user_id);
    return 0;
}
