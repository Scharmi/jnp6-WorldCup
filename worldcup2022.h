#ifndef WORLDCUP2022_H
#define WORLDCUP2022_H

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "worldcup.h"
class TooManyDiceException : public std::exception {};

class TooFewDiceException : public std::exception {};

class TooManyPlayersException : public std::exception {};

class TooFewPlayersException : public std::exception {};

class Dice {
   private:
    std::vector<std::shared_ptr<Die>> dice;
    unsigned int diceCount;

   public:
    constexpr Dice(int diceCount) : diceCount(diceCount) {}

    void addDie(std::shared_ptr<Die> die) {
        if (die != nullptr) {
            dice.push_back(die);
        }
    }

    constexpr int size() const { return dice.size(); }

    int roll() const {
        if (dice.size() < diceCount) {
            throw TooFewDiceException();
        }
        if (dice.size() > diceCount) {
            throw TooManyDiceException();
        }
        int sum = 0;
        for (auto die : dice) {
            sum += die->roll();
        }
        return sum;
    }
};

class Player {
   private:
    std::string const name;
    unsigned int money;
    unsigned int field;
    unsigned int suspension;
    bool bankrupted;

   public:
    constexpr Player(std::string const &name)
        : name(name), money(1000), field(0), suspension(0), bankrupted(false) {}

    constexpr bool bankrupt() const { return bankrupted; }

    constexpr bool waiting() const { return suspension > 0; }

    constexpr std::string getStatus() const {
        if (bankrupt()) {
            return "*** bankrut ***";
        } else if (waiting()) {
            return "*** czekanie: " + std::to_string(suspension) + " ***";
        } else {
            return "w grze";
        }
    }

    constexpr unsigned int getMoney() const { return money; }

    constexpr std::string getName() const { return name; }

    constexpr unsigned int getField() const { return field; }

    constexpr void waitIfNeeded() {
        if (suspension > 0) {
            suspension--;
        }
    }

    constexpr void suspend(unsigned int i) { suspension = i; }

    constexpr void move(unsigned int i) { field = i; }

    // Zak??adamy, ??e w przypadku braku wystarczaj??cej ilo??ci pieni??dzy gracz
    // p??aci wszystkie swoje pieni??dze
    constexpr int pay(unsigned int i) {
        if (money >= i) {
            money -= i;
            return i;
        } else {
            bankrupted = true;
            i = money;
            money = 0;
            return i;
        }
    }

    constexpr bool take(int i) {
        if (!bankrupted) {
            money += i;
            return true;
        }
        return false;
    }
};

class BoardField {
   protected:
    std::string const name;

   public:
    constexpr BoardField(std::string const &name) : name(name) {}

    virtual constexpr ~BoardField() = default;

    virtual constexpr void passField(Player *player) { (void)player; }

    virtual constexpr void landOnField(Player *player) { (void)player; }

    constexpr std::string getName() const { return name; }
};

class Beginning : public BoardField {
   private:
    unsigned int gift;

   public:
    constexpr Beginning(std::string const &name) : BoardField(name), gift(50) {}

    constexpr void passField(Player *player) override { player->take(gift); }

    constexpr void landOnField(Player *player) override { player->take(gift); }
};

class Goal : public BoardField {
   private:
    unsigned int bonus;

   public:
    constexpr Goal(std::string const &name, unsigned int bonus)
        : BoardField(name), bonus(bonus) {}

    constexpr void landOnField(Player *player) override { player->take(bonus); }
};

class Penalty : public BoardField {
   private:
    unsigned int fee;

   public:
    constexpr Penalty(std::string const &name, unsigned int fee)
        : BoardField(name), fee(fee) {}

    constexpr void landOnField(Player *player) override { player->pay(fee); }
};

class YellowCard : public BoardField {
   private:
    unsigned int suspension;

   public:
    constexpr YellowCard(std::string const &name, unsigned int suspension)
        : BoardField(name), suspension(suspension) {}

    constexpr void landOnField(Player *player) override {
        player->suspend(suspension);
    }
};

class Bookmaker : public BoardField {
   private:
    unsigned int bet;
    unsigned int cycle;
    unsigned int players;

   public:
    constexpr Bookmaker(std::string const &name, unsigned int bet)
        : BoardField(name), bet(bet), cycle(3), players(0) {}

    constexpr void landOnField(Player *player) override {
        if (players == 0) {
            player->take(bet);
        } else {
            player->pay(bet);
        }
        players = (players + 1) % cycle;
    }
};

class Match : public BoardField {
   private:
    unsigned int fee;
    double weight;
    unsigned int howMuchMoney;

   public:
    constexpr Match(std::string const &name, unsigned int fee, double weight)
        : BoardField(name), fee(fee), weight(weight), howMuchMoney(0) {}

    // Przechodzenie przez pole bez zatrzymania
    constexpr void passField(Player *player) override {
        howMuchMoney += player->pay(fee);
    }

    // Zatrzymanie na polu
    constexpr void landOnField(Player *player) override {
        if (player->take(howMuchMoney * weight)) howMuchMoney = 0;
    }
};

class EmptyField : public BoardField {
   public:
    constexpr EmptyField(std::string const &name) : BoardField(name) {}
};

class Board {
   private:
    std::vector<std::shared_ptr<BoardField>> fields;

   public:
    Board() : fields() {
        fields.push_back(std::make_shared<Beginning>("Pocz??tek sezonu"));
        fields.push_back(
            std::make_shared<Match>("Mecz z San Marino", 160, 1.0));
        fields.push_back(
            std::make_shared<EmptyField>("Dzie?? wolny od treningu"));
        fields.push_back(
            std::make_shared<Match>("Mecz z Liechtensteinem", 220, 1.0));
        fields.push_back(std::make_shared<YellowCard>("??????ta kartka", 3));
        fields.push_back(std::make_shared<Match>("Mecz z Meksykiem", 300, 2.5));
        fields.push_back(
            std::make_shared<Match>("Mecz z Arabi?? Saudyjsk??", 280, 2.5));
        fields.push_back(std::make_shared<Bookmaker>("Bukmacher", 100));
        fields.push_back(std::make_shared<Match>("Mecz z Argentyn??", 250, 2.5));
        fields.push_back(std::make_shared<Goal>("Gol", 120));
        fields.push_back(std::make_shared<Match>("Mecz z Francj??", 400, 4.0));
        fields.push_back(std::make_shared<Penalty>("Rzut karny", 180));
    }

    void playerMove(Player *player, unsigned int i) const {
        int currentField = player->getField();
        int nextField = (currentField + i) % fields.size();
        unsigned int counter = 0;
        while (counter + 1 < i) {
            fields[(currentField + counter + 1) % fields.size()]->passField(
                player);
            counter++;
        }
        player->move(nextField);
        fields[nextField]->landOnField(player);
    }

    std::string getFieldName(unsigned int i) const {
        return fields[i]->getName();
    }
};

class WorldCup2022 : public WorldCup {
   public:
    WorldCup2022() : scoreboard(), dice(2), players(), board() {}

    // destruktor
    ~WorldCup2022() {}

    // Je??eli argumentem jest pusty wska??nik, to nie wykonuje ??adnej operacji
    // (ale nie ma b????du).
    void addDie(std::shared_ptr<Die> die) override {
        if (die != nullptr) {
            dice.addDie(die);
        }
    }

    // Dodaje nowego gracza o podanej nazwie.
    void addPlayer(std::string const &name) override {
        std::shared_ptr<Player> player = std::make_shared<Player>(Player(name));
        players.push_back(player);
    }

    // Konfiguruje tablic?? wynik??w. Domy??lnie jest skonfigurowana tablica
    // wynik??w, kt??ra nic nie robi.
    void setScoreBoard(std::shared_ptr<ScoreBoard> scoreboard) override {
        if (scoreboard != nullptr) {
            this->scoreboard = scoreboard;
        }
    }

    // Przeprowadza rozgrywk?? co najwy??ej podanej liczby rund (rozgrywka mo??e
    // sko??czy?? si?? wcze??niej).
    // Jedna runda obejmuje po jednym ruchu ka??dego gracza.
    // Gracze ruszaj?? si?? w kolejno??ci, w kt??rej zostali dodani.
    // Na pocz??tku ka??dej rundy przekazywana jest informacja do tablicy wynik??w
    // o pocz??tku rundy (onRound), a na zako??czenie tury gracza informacje
    // podsumowuj??ce dla ka??dego gracza (onTurn).
    // Rzuca TooManyDiceException, je??li jest zbyt du??o kostek.
    // Rzuca TooFewDiceException, je??li nie ma wystarczaj??cej liczby kostek.
    // Rzuca TooManyPlayersException, je??li jest zbyt du??o graczy.
    // Rzuca TooFewPlayersException, je??li liczba graczy nie pozwala na
    // rozpocz??cie gry.
    // Wyj??tki powinny dziedziczy?? po std::exception.
    void play(unsigned int rounds) {
        if (players.size() > 11) {
            throw TooManyPlayersException();
            return;
        }
        if (players.size() < 2) {
            throw TooFewPlayersException();
            return;
        }

        unsigned int roundNumber = 0;

        while (roundNumber < rounds && players.size() > 1) {
            scoreboard->onRound(roundNumber);
            //
            for (size_t i = 0; i < players.size();) {
                Player *player = players[i].get();
                player->waitIfNeeded();
                // Sprawdzenie czy gracz nie pauzuje
                if (!player->waiting()) {
                    int diceResult = dice.roll();
                    board.playerMove(player, diceResult);
                }
                scoreboard->onTurn(player->getName(), player->getStatus(),
                                   board.getFieldName(player->getField()),
                                   player->getMoney());
                if (player->bankrupt()) {
                    players.erase(players.begin() + i);
                    // Je??li pozosta?? tylko jeden gracz, nast??puje zako??czenie
                    // gry
                    if (players.size() == 1) break;
                    // W przypadku usuwania gracza nie trzeba zwi??ksza?? indeksu
                    // gdy?? nast??pny gracz zast??pi go na aktualnej pozycji
                } else {
                    i++;
                }
            }
            roundNumber++;
        }

        // Sprawdzenie kto wygra?? w przypadku gdy po rozegraniu wszystkich rund
        // zosta?? wi??cej ni?? jeden gracz
        Player *winner = players[0].get();

        for (auto player : players) {
            if (player->getMoney() > winner->getMoney()) {
                winner = player.get();
            }
        }

        scoreboard->onWin(winner->getName());
    }

   private:
    std::shared_ptr<ScoreBoard> scoreboard;
    Dice dice;
    std::vector<std::shared_ptr<Player>> players;
    Board board;
};

#endif