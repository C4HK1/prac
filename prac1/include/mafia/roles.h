#pragma once

#include <string>
#include <string_view>

enum class Class { Unknown,
                   Civilian,
                   Mafia,
                   Neutral };

enum class Role { Unknown,
                  Civilian,
                  Mafia,
                  Don,
                  Kamikadze,
                  Commissioner,
                  Doctor,
                  Maniac,
                  Journalist,
                  Hacker };

inline constexpr Class get_faction(Role kind) {
    switch (kind) {
        case Role::Mafia:
        case Role::Don:
        case Role::Kamikadze:
            return Class::Mafia;
        case Role::Maniac:
        case Role::Hacker:
            return Class::Neutral;
        case Role::Civilian:
        case Role::Commissioner:
        case Role::Doctor:
        case Role::Journalist:
            return Class::Civilian;
        default:
            return Class::Unknown;
    }
}

inline std::string role_name(Role kind) {
    switch (kind) {
        case Role::Civilian:
            return "Мирный";
        case Role::Mafia:
            return "Мафия";
        case Role::Don:
            return "Дон";
        case Role::Journalist:
            return "Журналист";
        case Role::Commissioner:
            return "Комиссар";
        case Role::Doctor:
            return "Доктор";
        case Role::Maniac:
            return "Маньяк";
        case Role::Hacker:
            return "Хакер";
        case Role::Kamikadze:
            return "Камикадзе";
        default:
            return "";
    }
}
