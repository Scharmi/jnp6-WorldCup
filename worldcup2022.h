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
        Dice(int diceCount) {
            this->diceCount = diceCount;
        }
        void addDie(std::shared_ptr<Die> die) {
            if (die != nullptr) {
                dice.push_back(die);
            }
        }
        int size() {
            return dice.size();
        }
        int roll() {
            if(dice.size() < diceCount) {
                throw TooFewDiceException();
            }
            if(dice.size() > diceCount) {
                throw TooManyDiceException();
            }
            int sum = 0;
            for (auto die: dice) {
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

   public:
    Player(std::string const &name)
        : name(name), money(1000), field(0), suspension(0) {}

    bool bankrupt() { return money == 0; }

    bool waiting() { return suspension > 0; }

    std::string getStatus() {
        if (bankrupt()) {
            return "*** bankrut ***";
        } else if (waiting()) {
            return "*** czekanie: " + std::to_string(suspension) + " ***";
        } else {
            return "w grze";
        }
    }

    unsigned int getMoney() { return money; }

    std::string getName() { return name; }

    unsigned int getField() { return field; }

    void waitIfNeeded() {
        if (suspension > 0) {
            suspension--;
        }
    }

    void suspend(unsigned int i) { suspension = i; }

    void move(unsigned int i) { field = i; }

    bool pay(unsigned int i) {
        if (money >= i) {
            money -= i;
            return true;
        } else {
            money = 0;
            return false;
        }
    }

    void take(int i) { money += i; }
};

class BoardField {
   protected:
    std::string const name;

   public:
    BoardField(std::string const &name) : name(name) {}

    virtual ~BoardField() = default;

    virtual void passField(Player *player) { (void) player; }

    virtual void landOnField(Player *player) { (void) player; }

    std::string getName() { return name; }
};

class Beginning : public BoardField {
   private:
    unsigned int gift;

   public:
    Beginning(std::string const &name) : BoardField(name), gift(50) {}

    void passField(Player *player) override { player->take(gift); }

    void landOnField(Player *player) override { player->take(gift); }
};

class Goal : public BoardField {
   private:
    unsigned int bonus;

   public:
    Goal(std::string const &name, unsigned int bonus)
        : BoardField(name), bonus(bonus) {}

    void landOnField(Player *player) override { player->take(bonus); }
};

class Penalty : public BoardField {
   private:
    unsigned int fee;

   public:
    Penalty(std::string const &name, unsigned int fee)
        : BoardField(name), fee(fee) {}

    void landOnField(Player *player) override { player->pay(fee); }
};

class YellowCard : public BoardField {
   private:
    unsigned int suspension;

   public:
    YellowCard(std::string const &name, unsigned int suspension)
        : BoardField(name), suspension(suspension) {}

    void landOnField(Player *player) override { player->suspend(suspension); }
};

class Bookmaker : public BoardField {
   private:
    unsigned int bet;
    unsigned int cycle;
    unsigned int players;

   public:
    Bookmaker(std::string const &name, unsigned int bet)
        : BoardField(name), bet(bet), cycle(3), players(0) {}

    void landOnField(Player *player) override {
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
    unsigned int howManyFees;

   public:
    Match(std::string const &name, unsigned int fee, double weight)
        : BoardField(name), fee(fee), weight(weight), howManyFees(0) {}

    ~Match() override = default;
    //Przechodzenie przez pole bez zatrzymania
    void passField(Player *player) override {
        if (player->pay(fee)) howManyFees++;
    }
    //Zatrzymanie na polu
    void landOnField(Player *player) override {
        player->take(fee * weight * howManyFees);
        howManyFees = 0;
    }
};

class EmptyField : public BoardField {
   public:
    EmptyField(std::string const &name) : BoardField(name) {}
};

class Board {
   private:
    std::vector<std::shared_ptr<BoardField>> fields;

   public:
    Board() : fields() {
        fields.push_back(std::make_shared<Beginning>("Początek sezonu"));
        fields.push_back(
            std::make_shared<Match>("Mecz z San Marino", 160, 1.0));
        fields.push_back(
            std::make_shared<EmptyField>("Dzień wolny od treningu"));
        fields.push_back(
            std::make_shared<Match>("Mecz z Liechtensteinem", 220, 1.0));
        fields.push_back(std::make_shared<YellowCard>("Żółta kartka", 3));
        fields.push_back(std::make_shared<Match>("Mecz z Meksykiem", 300, 2.5));
        fields.push_back(
            std::make_shared<Match>("Mecz z Arabią Saudyjską", 280, 2.5));
        fields.push_back(std::make_shared<Bookmaker>("Bukmacher", 100));
        fields.push_back(std::make_shared<Match>("Mecz z Argentyną", 250, 2.5));
        fields.push_back(std::make_shared<Goal>("Gol", 120));
        fields.push_back(std::make_shared<Match>("Mecz z Francją", 400, 4.0));
        fields.push_back(std::make_shared<Penalty>("Rzut karny", 180));
    }

    void playerMove(Player *player, unsigned int i) {
        int currentField = player->getField();
        int nextField = (currentField + i) % fields.size();
        // Przechodzenie przez kolejne pola
        for (int j = (currentField + 1) % fields.size(); j != nextField;
             j = (j + 1) % fields.size()) {
            fields[j]->passField(player);
        }
        player->move(nextField);
        fields[nextField]->landOnField(player);
    }

    std::string getFieldName(unsigned int i) { return fields[i]->getName(); }
};



class WorldCup2022 : public WorldCup {
   public:
    WorldCup2022() : scoreboard(), dice(2), players(), board() {}

    // destruktor
    ~WorldCup2022() {}

    // Jeżeli argumentem jest pusty wskaźnik, to nie wykonuje żadnej operacji
    // (ale nie ma błędu).
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

    // Konfiguruje tablicę wyników. Domyślnie jest skonfigurowana tablica
    // wyników, która nic nie robi.
    void setScoreBoard(std::shared_ptr<ScoreBoard> scoreboard) override {
        if (scoreboard != nullptr) {
            this->scoreboard = scoreboard;
        }
    }

    // Przeprowadza rozgrywkę co najwyżej podanej liczby rund (rozgrywka może
    // skończyć się wcześniej).
    // Jedna runda obejmuje po jednym ruchu każdego gracza.
    // Gracze ruszają się w kolejności, w której zostali dodani.
    // Na początku każdej rundy przekazywana jest informacja do tablicy wyników
    // o początku rundy (onRound), a na zakończenie tury gracza informacje
    // podsumowujące dla każdego gracza (onTurn).
    // Rzuca TooManyDiceException, jeśli jest zbyt dużo kostek.
    // Rzuca TooFewDiceException, jeśli nie ma wystarczającej liczby kostek.
    // Rzuca TooManyPlayersException, jeśli jest zbyt dużo graczy.
    // Rzuca TooFewPlayersException, jeśli liczba graczy nie pozwala na
    // rozpoczęcie gry.
    // Wyjątki powinny dziedziczyć po std::exception.
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
            for (size_t i = 0; i < players.size(); ) {
                Player *player = players[i].get();
                player->waitIfNeeded();
                //Sprawdzenie czy gracz nie pauzuje
                if (!player->waiting()) {
                    int diceResult = dice.roll();
                    board.playerMove(player, diceResult);
                }
                scoreboard->onTurn(player->getName(), player->getStatus(),
                                   board.getFieldName(player->getField()),
                                   player->getMoney());
                if (player->bankrupt()) {
                    players.erase(players.begin() + i);
                    //Jeśli pozostał tylko jeden gracz, następuje zakończenie gry
                    if (players.size() == 1) 
                        break;
                    //W przypadku usuwania gracza nie trzeba zwiększać indeksu gdyż następny gracz zastąpi go na aktualnej pozycji
                } else {
                    i++;
                }
            }
            roundNumber++;
        }
        //Sprawdzenie kto wygrał w przypadku gdy po rozegraniu wszystkich rund został więcej niż jeden gracz
        Player *winner = players[0].get();

        for (auto player: players) {
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