#include "worldcup2022.h"

#include <sstream>
#include <memory>
#include <string>
#include <cassert>
#include <regex>
#include <vector>
#include <cwctype>
#include <codecvt>
#include <iostream>

namespace dice {
    using roll_t = unsigned short;
    using rolls_t = std::vector<roll_t>;

    class ZeroDie : public Die {
    public:
        [[nodiscard]] roll_t roll() const override {
            return 0;
        }
    };

    const rolls_t DEFAULT_ROLLS = {1, 1, 1, 2, 1, 3};

    class FixedDie : public Die {
        rolls_t _rolls;

        static rolls_t multiply(rolls_t const &rolls, size_t multiplier) {
            rolls_t ret;
            for (auto roll: rolls) {
                for (size_t i = 0; i < multiplier; i++) {
                    ret.push_back(roll);
                }
            }
            return ret;
        }

    public:
        FixedDie() : FixedDie(DEFAULT_ROLLS) {}

        explicit FixedDie(rolls_t rolls) : _rolls(std::move(rolls)) {}

        FixedDie(rolls_t const &rolls, size_t multiplier) : FixedDie(multiply(rolls, multiplier)) {}

        [[nodiscard]] roll_t roll() const override {
            static size_t current = 0;
            auto ret = _rolls[current];
            current = (current + 1) % _rolls.size();
            return ret;
        }
    };
} // namespace dice

namespace text {
    std::string replace(std::string const &str, std::string const &src, std::string const &dst);

    std::string normalize(std::string const &str);

    class Result {
        std::string _actual;

    public:
        explicit Result(std::string const &str) : _actual(normalize(str)) {}

        void equals(std::string const &expectedResult) {
            std::string expected = normalize(expectedResult);
            assert(_actual == expected ||
                   !(std::cerr << "ACTUAL" << std::endl << _actual << std::endl
                               << "EXPECTED" << std::endl << expected << std::endl));
        }

        void equals(std::string const &expectedResult1, std::string const &expectedResult2) {
            std::string expected1 = normalize(expectedResult1);
            std::string expected2 = normalize(expectedResult2);
            assert(_actual == expected1 || _actual == expected2 ||
                   !(std::cerr << "ACTUAL" << std::endl << _actual << std::endl
                               << "EXPECTED v1" << std::endl << expected1 << std::endl
                               << "EXPECTED v2" << std::endl << expected2 << std::endl));
        }

        Result& ignoreWinner() {
            std::size_t winner = _actual.find("=== zwyc");
            if (winner != std::string::npos) {
                _actual = _actual.substr(0, winner);
            }
            return *this;
        }

        Result& lastRound() {
            std::size_t round = _actual.rfind("runda");
            if (round != std::string::npos) {
                _actual = _actual.substr(round, _actual.length() - round);
                _actual = replace(_actual, "runda(.+)\\n", "");
            }
            return *this;
        }
    };

    class TextScoreBoard : public ScoreBoard {
        std::stringstream _info;

    public:
        void onRound(unsigned int roundNo) override {
            _info << "=== Runda: " << roundNo << "\n";
        }

        void onTurn(std::string const &playerName, std::string const &playerStatus,
                    std::string const &currentSquareName, unsigned int currentCash) override {
            _info << playerName << " [" << playerStatus << "] [" << currentCash << "] - "
                  << currentSquareName << "\n";
        }

        void onWin(const std::string& playerName) override {
            _info << "=== Zwycięzca: " << playerName << "\n";
        }

        std::string str() {
            return _info.str();
        }

        Result result() {
            return Result(str());
        }
    };

    std::wstring toWideString(std::string const &str) {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
    }

    std::string toString(std::wstring const &wstr) {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
    }

    std::string toLowerCase(std::string const &str) {
        static const std::locale PL("pl_PL.UTF-8");
        // a hack to properly lowercase PL
        std::wstring wstr = toWideString(str);
        std::transform(wstr.begin(), wstr.end(), wstr.begin(),
                       [](wchar_t c) { return std::tolower(c, PL); });
        return toString(wstr);
    }

    std::string replace(std::string const &str, std::string const &src, std::string const &dst) {
        return std::regex_replace(str, std::regex(src), dst);
    }

    std::string normalize(std::string const &str) {
        std::string ret = toLowerCase(str);
        return replace(ret, "lichtenstein", "liechtenstein");;
    }
} // namespace text

namespace worldcup {
    using die_ptr_t = std::shared_ptr<Die>;
    using dice_t = std::vector<die_ptr_t>;
    using names_t = std::vector<std::string>;
    using worldcup_ptr_t = std::shared_ptr<WorldCup>;
    using scoreboard_ptr_t = std::shared_ptr<text::TextScoreBoard>;

    class Builder {
        dice_t _dice;
        names_t _names;
        scoreboard_ptr_t _scoreboard;

        void addDice(worldcup_ptr_t const &worldcup) {
            for (auto &die: _dice) {
                worldcup->addDie(die);
            }
        }

        void addPlayers(worldcup_ptr_t const &worldcup) {
            for (auto &name: _names) {
                worldcup->addPlayer(name);
            }
        }

        void setScoreboard(worldcup_ptr_t const &worldcup) {
            _scoreboard = std::make_shared<text::TextScoreBoard>();
            worldcup->setScoreBoard(_scoreboard);
        }

        void setDice(dice::rolls_t const &rolls, size_t multiplier) {
            auto die1 = std::make_shared<dice::FixedDie>(rolls, multiplier);
            auto die2 = std::make_shared<dice::ZeroDie>();
            _dice.push_back(die1);
            _dice.push_back(die2);
        }

    public:
        Builder& dice(size_t num) {
            while (num-- > 0) {
                std::shared_ptr<Die> die = std::make_shared<dice::ZeroDie>();
                _dice.push_back(die);
            }
            return *this;
        }

        Builder& players(size_t num) {
            for (size_t i = 1; i <= num; i++) {
                std::string name = "Player-" + std::to_string(i);
                _names.push_back(name);
            }
            return *this;
        }

        Builder& rollsPerPlayer(dice::rolls_t const &rolls) {
            setDice(rolls, _names.size());
            return *this;
        }

        Builder& rolls(std::vector<unsigned short> const &rolls) {
            setDice(rolls, 1);
            return *this;
        }

        scoreboard_ptr_t getScoreboard() {
            return _scoreboard;
        }

        worldcup_ptr_t build() {
            std::shared_ptr<WorldCup> ret = std::make_shared<WorldCup2022>();
            addDice(ret);
            addPlayers(ret);
            setScoreboard(ret);
            return ret;
        }
    };

    using fields_t = std::vector<std::string>;
    using indices_t = std::vector<size_t>;

    struct Fields {
        static constexpr const char *POCZATEK_SEZONU = "początek sezonu";
        static constexpr const char *MECZ_SAN_MARINO = "mecz z San Marino";
        static constexpr const char *DZIEN_WOLNY = "dzień wolny od treningu";
        static constexpr const char *MECZ_LIECHTENSTEIN = "mecz z Liechtensteinem";
        static constexpr const char *ZOLTA_KARTKA = "żółta kartka";
        static constexpr const char *MECZ_MEKSYK = "mecz z Meksykiem";
        static constexpr const char *MECZ_ARABIA_SAUDYJSKA = "mecz z Arabią Saudyjską";
        static constexpr const char *BUKMACHER = "bukmacher";
        static constexpr const char *MECZ_ARGENTYNA = "mecz z Argentyną";
        static constexpr const char *GOL = "gol";
        static constexpr const char *MECZ_FRANCJA = "mecz z Francją";
        static constexpr const char *RZUT_KARNY = "rzut karny";

        static const fields_t ALL;

        static const indices_t MATCH_FIELDS;

        static size_t getIndex(std::string const &name) {
            for (size_t i = 0; i < ALL.size(); i++) {
                if (name == ALL[i]) return i;
            }
            throw std::out_of_range(name);
        }

        static size_t getIndex(size_t fieldNo) {
            return fieldNo % ALL.size();
        }

        static std::string getName(size_t fieldNo) {
            return ALL[fieldNo % ALL.size()];
        }

        static size_t stepsBetween(size_t from, size_t to) {
            if (from == to) return ALL.size();
            if (from > to) return ALL.size() - from + to;
            return to - from;
        }

        static size_t findMatchBefore(size_t fieldNo) {
            size_t size = MATCH_FIELDS.size();
            for (size_t i = 0; i < size; i++) {
                size_t matchField = MATCH_FIELDS[size - i - 1];
                if (matchField < fieldNo) return matchField;
            }
            return 0;
        }

        static size_t findMatchAfter(size_t fieldNo) {
            for (auto matchField: MATCH_FIELDS) {
                if (matchField > fieldNo) return matchField;
            }
            return ALL.size() + MATCH_FIELDS[0];
        }

        static size_t findStepsTillMatch(size_t cur, size_t& curMatch) {
            auto curFieldNo = getIndex(cur);
            auto matchFieldNo = MATCH_FIELDS[curMatch];
            curMatch = (curMatch + 1) % MATCH_FIELDS.size();
            return stepsBetween(curFieldNo, matchFieldNo);
        }

        // jak skaczemy po meczach to nie tracimy zdzisławów
        static dice::rolls_t calculateNoLossRolls(size_t to) {
            dice::rolls_t ret;
            size_t final = to == 0 ? ALL.size() : to;
            size_t cur = 0;
            size_t curMatch = 0;
            auto step = findStepsTillMatch(cur, curMatch);
            while (cur + step < final) {
                ret.push_back(step);
                cur += step;
                step = findStepsTillMatch(cur, curMatch);
            }
            if (final > cur) {
                step = final - cur;
                ret.push_back(step);
            }
            return ret;
        }
    };

    const fields_t Fields::ALL = {
            Fields::POCZATEK_SEZONU,
            Fields::MECZ_SAN_MARINO,
            Fields::DZIEN_WOLNY,
            Fields::MECZ_LIECHTENSTEIN,
            Fields::ZOLTA_KARTKA,
            Fields::MECZ_MEKSYK,
            Fields::MECZ_ARABIA_SAUDYJSKA,
            Fields::BUKMACHER,
            Fields::MECZ_ARGENTYNA,
            Fields::GOL,
            Fields::MECZ_FRANCJA,
            Fields::RZUT_KARNY
    };

    const indices_t Fields::MATCH_FIELDS = {2 - 1, 4 - 1, 6 - 1, 7 - 1, 9 - 1, 11 - 1};

    class Test {
        static std::string expectedRound(std::string const &fieldName,
                                         std::string const &playerStatus,
                                         std::vector<unsigned> const &playerMoneys) {
            static const std::string TEMPLATE_LINE = "Player-NUMBER [STATUS] [MONEY] - FIELD_NAME\n";
            std::string ret;
            for (size_t i = 0; i < playerMoneys.size(); i++) {
                std::string line = text::replace(TEMPLATE_LINE, "NUMBER", std::to_string(i + 1));
                line = text::replace(line, "FIELD_NAME", fieldName);
                line = text::replace(line, "STATUS", playerStatus);
                line = text::replace(line, "MONEY", std::to_string(playerMoneys[i]));
                ret += line;
            }
            return ret;
        }

    public:
        static void testGameplay(dice::rolls_t rolls,
                                 size_t extraRounds,
                                 size_t expectedField,
                                 std::vector<unsigned> const &expectedCash,
                                 std::string const &expectedStatus) {
            auto playersNum = expectedCash.size();
            auto builder = worldcup::Builder();
            auto worldcup2022 = builder
                    .players(playersNum)
                    .rollsPerPlayer(rolls)
                    .build();
            auto scoreboard = builder.getScoreboard();

            auto rounds = rolls.size() + extraRounds;
            worldcup2022->play(rounds);

            auto expectedLastRound = expectedRound(Fields::getName(expectedField), expectedStatus, expectedCash);
            scoreboard->result().lastRound().ignoreWinner().equals(expectedLastRound);
        }

        static void testLandingOnField(unsigned short fieldNo,
                                       size_t extraRounds,
                                       std::string const &expectedStatus,
                                       std::vector<unsigned> const &expectedCash) {
            auto rolls = Fields::calculateNoLossRolls(fieldNo);
            testGameplay(rolls, extraRounds, fieldNo, expectedCash, expectedStatus);
        }

        static void testLandingOnField(unsigned short fieldNo,
                                       std::string const &playerStatus,
                                       unsigned playerMoney) {
            testLandingOnField(fieldNo, 0, playerStatus, {playerMoney, playerMoney});
        }

        static void testLandingOnField(unsigned short fieldNo,
                                       std::string const &playerStatus,
                                       unsigned playerMoney,
                                       unsigned extraRounds) {
            testLandingOnField(fieldNo, extraRounds, playerStatus, {playerMoney, playerMoney});
        }

        static void testPassingField(unsigned short fieldNo,
                                     unsigned playerMoney) {
            auto finalField = Fields::findMatchAfter(fieldNo);;
            if (fieldNo == 0) finalField += Fields::ALL.size();
            testLandingOnField(Fields::stepsBetween(0, finalField), "w grze", playerMoney);
        }

        static void testPassingMatchField(unsigned short fieldNo,
                                          unsigned playerMoney) {
            auto matchBefore = Fields::findMatchBefore(fieldNo);
            auto matchAfter = Fields::findMatchAfter(fieldNo);
            dice::rolls_t rolls;
            if (matchBefore != 0) rolls = Fields::calculateNoLossRolls(matchBefore);
            rolls.push_back(Fields::stepsBetween(matchBefore, matchAfter));
            testGameplay(rolls, 0, matchAfter, {playerMoney, playerMoney}, "w grze");
        }

        static void testException(size_t numOfDice, size_t numOfPlayers) {
            auto worldCup2022 = worldcup::Builder()
                    .dice(numOfDice)
                    .players(numOfPlayers)
                    .build();

            try {
                worldCup2022->play(100);
                assert(false);
            } catch (std::exception const &e) {
            }
        }
    };
} // namespace worldcup

int main() {

// Test z treści zadania
#if TEST_NUM == 100
    std::shared_ptr<Die> die1 = std::make_shared<dice::FixedDie>();
    std::shared_ptr<Die> die2 = std::make_shared<dice::FixedDie>();
    std::shared_ptr<text::TextScoreBoard> scoreboard = std::make_shared<text::TextScoreBoard>();

    std::shared_ptr<WorldCup> worldCup2022 = std::make_shared<WorldCup2022>();
    worldCup2022->addDie(die1);
    worldCup2022->addDie(die2);
    worldCup2022->addPlayer("Lewandowski");
    worldCup2022->addPlayer("Messi");
    worldCup2022->addPlayer("Ronaldo");
    worldCup2022->setScoreBoard(scoreboard);

    worldCup2022->play(100);

    scoreboard->result().equals("=== Runda: 0\n"
                                "Lewandowski [w grze] [840] - Dzień wolny od treningu\n"
                                "Messi [w grze] [840] - Mecz z Liechtensteinem\n"
                                "Ronaldo [*** czekanie: 3 ***] [620] - Żółta kartka\n"
                                "=== Runda: 1\n"
                                "Lewandowski [*** czekanie: 3 ***] [620] - Żółta kartka\n"
                                "Messi [w grze] [540] - Mecz z Arabią Saudyjską\n"
                                "Ronaldo [*** czekanie: 2 ***] [620] - Żółta kartka\n"
                                "=== Runda: 2\n"
                                "Lewandowski [*** czekanie: 2 ***] [620] - Żółta kartka\n"
                                "Messi [w grze] [290] - Mecz z Francją\n"
                                "Ronaldo [*** czekanie: 1 ***] [620] - Żółta kartka\n"
                                "=== Runda: 3\n"
                                "Lewandowski [*** czekanie: 1 ***] [620] - Żółta kartka\n"
                                "Messi [w grze] [340] - Początek sezonu\n"
                                "Ronaldo [w grze] [140] - Bukmacher\n"
                                "=== Runda: 4\n"
                                "Lewandowski [w grze] [665] - Mecz z Argentyną\n"
                                "Messi [w grze] [180] - Dzień wolny od treningu\n"
                                "Ronaldo [*** bankrut ***] [0] - Mecz z Francją\n"
                                "=== Runda: 5\n"
                                "Lewandowski [w grze] [315] - Początek sezonu\n"
                                "Messi [*** bankrut ***] [0] - Żółta kartka\n"
                                "=== Zwycięzca: Lewandowski\n");
#endif

// 2xx Testy wyjątków

// TooFewPlayersException
#if TEST_NUM == 200
    worldcup::Test::testException(2, 1);
#endif

// TooManyPlayersException
#if TEST_NUM == 201
    worldcup::Test::testException(2, 12);
#endif

// TooFewDiceException
#if TEST_NUM == 202
    worldcup::Test::testException(1, 2);
#endif

// TooManyDiceException
#if TEST_NUM == 203
    worldcup::Test::testException(3, 2);
#endif

// 3xx Testy pól: początek sezonu, dzień wolny od treningu, gol, rzut karny

#if TEST_NUM == 300
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::POCZATEK_SEZONU);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1050);
    worldcup::Test::testPassingField(fieldNo, 1050);
#endif

#if TEST_NUM == 301
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::DZIEN_WOLNY);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1000);
    worldcup::Test::testPassingField(fieldNo, 1000);
#endif

#if TEST_NUM == 302
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::GOL);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1120);
    worldcup::Test::testPassingField(fieldNo, 1000);
#endif

#if TEST_NUM == 303
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::RZUT_KARNY);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 820);
    worldcup::Test::testPassingField(fieldNo, 1050);
#endif

// 4xx Testy pól: bukmacher, żółta kartka

#if TEST_NUM == 400
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::BUKMACHER);
    worldcup::Test::testLandingOnField(fieldNo, 0, "w grze", {1100, 900, 900, 1100});
    worldcup::Test::testPassingField(fieldNo, 1000);
#endif

#if TEST_NUM == 401
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::ZOLTA_KARTKA);
    worldcup::Test::testLandingOnField(fieldNo, "*** czekanie: 3 ***", 1000);
    worldcup::Test::testLandingOnField(fieldNo, "*** czekanie: 2 ***", 1000, 1);
    worldcup::Test::testLandingOnField(fieldNo, "*** czekanie: 1 ***", 1000, 2);
    worldcup::Test::testPassingField(fieldNo, 1000);
#endif

#if TEST_NUM == 402
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::ZOLTA_KARTKA);
    worldcup::Test::testLandingOnField(fieldNo, "*** czekanie: 3 ***", 1000);
    worldcup::Test::testLandingOnField(fieldNo, "*** czekanie: 2 ***", 1000, 1);
    worldcup::Test::testLandingOnField(fieldNo, "*** czekanie: 1 ***", 1000, 2);
    worldcup::Test::testPassingField(fieldNo, 1000);
#endif

// 5xx Testy meczu oraz różnych kombinacji meczy

#if TEST_NUM == 500
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::MECZ_SAN_MARINO);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1000);
    worldcup::Test::testPassingMatchField(fieldNo, 840);
#endif

#if TEST_NUM == 501
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::MECZ_LIECHTENSTEIN);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1000);
    worldcup::Test::testPassingMatchField(fieldNo, 780);
#endif

#if TEST_NUM == 502
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::MECZ_MEKSYK);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1000);
    worldcup::Test::testPassingMatchField(fieldNo, 700);
#endif

#if TEST_NUM == 503
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::MECZ_ARABIA_SAUDYJSKA);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1000);
    worldcup::Test::testPassingMatchField(fieldNo, 720);
#endif

#if TEST_NUM == 504
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::MECZ_ARGENTYNA);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1000);
    worldcup::Test::testPassingMatchField(fieldNo, 750);
#endif

#if TEST_NUM == 505
    auto fieldNo = worldcup::Fields::getIndex(worldcup::Fields::MECZ_FRANCJA);
    worldcup::Test::testLandingOnField(fieldNo, "w grze", 1000);
    // przechodzi przez początek sezonu dlatego -400 + 50
    worldcup::Test::testPassingMatchField(fieldNo, 650);
#endif

#if TEST_NUM == 506
    auto builder = worldcup::Builder();
    auto worldcup2022 = builder
            .players(2)
            .rolls({2, 1})
            .build();

    worldcup2022->play(1);

    builder.getScoreboard()->result().ignoreWinner().equals(
                     "=== Runda: 0\n"
                     "Player-1 [w grze] [840] - Dzień wolny od treningu\n"
                     "Player-2 [w grze] [1160] - Mecz z San Marino\n");
#endif

#if TEST_NUM == 507
    auto builder = worldcup::Builder();
    auto worldcup2022 = builder
            .players(2)
            .rolls({2, 1, 1, 2, 2, 5, 1, 4, 4, 1})
            .build();

    worldcup2022->play(5);

    builder.getScoreboard()->result().equals(
                     "=== Runda: 0\n"
                     "Player-1 [w grze] [840] - Dzień wolny od treningu\n"
                     "Player-2 [w grze] [1160] - Mecz z San Marino\n"
                     "=== Runda: 1\n"
                     "Player-1 [w grze] [840] - Mecz z Lichtensteinem\n"
                     "Player-2 [w grze] [1160] - Mecz z Lichtensteinem\n"
                     "=== Runda: 2\n"
                     "Player-1 [w grze] [840] - Mecz z Meksykiem\n"
                     "Player-2 [w grze] [580] - Mecz z Argentyną\n"
                     "=== Runda: 3\n"
                     "Player-1 [w grze] [1540] - Mecz z Arabią Saudyjską\n"
                     "Player-2 [w grze] [230] - Początek sezonu\n"
                     "=== Runda: 4\n"
                     "Player-1 [w grze] [2890] - Mecz z Francją\n"
                     "Player-2 [w grze] [230] - Mecz z San Marino\n");
#endif

// 6xx Różne kombinacje, bankructwo

// szybkie bankructwo pierwszego gracza
#if TEST_NUM == 600
    auto builder = worldcup::Builder();
    auto worldcup2022 = builder
            .players(2)
            .rolls({12, 1})
            .build();

    worldcup2022->play(5);

    builder.getScoreboard()->result().equals("=== Runda: 0\n"
                                             "Player-1 [*** bankrut ***] [0] - Początek sezonu\n"
                                             "=== Zwycięzca: Player-2\n");
#endif

// szybkie bankructwo drugiego gracza
#if TEST_NUM == 601
    auto builder = worldcup::Builder();
    auto worldcup2022 = builder
            .players(2)
            .rolls({1, 12})
            .build();

    worldcup2022->play(5);

    builder.getScoreboard()->result().equals(
            "=== Runda: 0\n"
            "Player-1 [w grze] [1000] - Mecz z San Marino\n"
            "Player-2 [*** bankrut ***] [0] - Początek sezonu\n"
            "=== Zwycięzca: Player-1\n");
#endif

#if TEST_NUM == 602
    auto builder = worldcup::Builder();
    auto worldcup2022 = builder
            .players(2)
            .rolls({1, 1, 2, 2, 1, 2, 1, 1, 2})
            .build();

    worldcup2022->play(100);

    builder.getScoreboard()->result().equals(
            "=== Runda: 0\n"
            "Player-1 [w grze] [1000] - Mecz z San Marino\n"
            "Player-2 [w grze] [1000] - Mecz z San Marino\n"
            "=== Runda: 1\n"
            "Player-1 [w grze] [1000] - Mecz z Lichtensteinem\n"
            "Player-2 [w grze] [1000] - Mecz z Lichtensteinem\n"
            "=== Runda: 2\n"
            "Player-1 [*** czekanie: 3 ***] [1000] - Żółta kartka\n"
            "Player-2 [w grze] [1000] - Mecz z Meksykiem\n"
            "=== Runda: 3\n"
            "Player-1 [*** czekanie: 2 ***] [1000] - Żółta kartka\n"
            "Player-2 [w grze] [1000] - Mecz z Arabią Saudyjską\n"
            "=== Runda: 4\n"
            "Player-1 [*** czekanie: 1 ***] [1000] - Żółta kartka\n"
            "Player-2 [w grze] [1100] - Bukmacher\n"
            "=== Runda: 5\n"
            "Player-1 [w grze] [700] - Mecz z Arabią Saudyjską\n"
            "Player-2 [w grze] [1100] - Mecz z Argentyną\n"
            "=== Runda: 6\n"
            "Player-1 [w grze] [600] - Bukmacher\n"
            "Player-2 [w grze] [1100] - Mecz z Francją\n"
            "=== Runda: 7\n"
            "Player-1 [w grze] [470] - Gol\n"
            "Player-2 [w grze] [920] - Rzut karny\n"
            "=== Runda: 8\n"
            "Player-1 [*** bankrut ***] [0] - Rzut karny\n"
            "=== Zwycięzca: Player-2\n");
#endif

#if TEST_NUM == 603
    auto builder = worldcup::Builder();
    auto worldcup2022 = builder
            .players(2)
            .rolls({1, 1})
            .build();

    worldcup2022->play(1);

    // Dopuszczalny jest brak zwycięzców lub dwóch zwycięzców.
    builder.getScoreboard()->result().equals(
            "=== runda: 0\n"
            "Player-1 [w grze] [1000] - Mecz z San Marino\n"
            "Player-2 [w grze] [1000] - Mecz z San Marino\n",
            "=== runda: 0\n"
            "Player-1 [w grze] [1000] - Mecz z San marino\n"
            "Player-2 [w grze] [1000] - Mecz z San marino\n"
            "=== Zwycięzca: Player-1\n"
            "=== Zwycięzca: Player-2\n");
#endif
}
