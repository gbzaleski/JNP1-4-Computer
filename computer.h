// Franciszek Biel (fb123456)
//  & Grzegorz B. Zaleski (gz418494)

#ifndef COMPUTER_H
#define COMPUTER_H

#include <cstdint>
#include <array>
#include <type_traits>

using Id_t = uint64_t;

struct Instruction {
};
struct R_value {
};

template<typename Arg>
constexpr static void is_r_value() {
    static_assert(std::is_base_of<R_value, Arg>::value, "R-VALUE WAS EXPECTED, BUT NOT PROVIDED");
}


constexpr Id_t Id(const char *id) {
    const Id_t NUMB_OF_DIGITS = 10;
    const Id_t SYSTEM_BASE = 40;
    const size_t MAX_STR_LENGTH = 6;

    std::string_view str(id);
    if (str.empty() || str.size() > MAX_STR_LENGTH)
        throw "Wrong length of ID";


    Id_t res = 1;
    for (auto c : str) {

        if ('0' <= c && c <= '9') // cyfra
            res = res * SYSTEM_BASE + c - '0';
        else if ('a' <= c && c <= 'z') // mała litera
            res = res * SYSTEM_BASE + c - 'a' + NUMB_OF_DIGITS;
        else if ('A' <= c && c <= 'Z') // wielka litera
            res = res * SYSTEM_BASE + c - 'A' + NUMB_OF_DIGITS;
        else
            throw "Wrong sign!";

    }

    return res;
}

template<auto V>
struct Num : R_value {
    template<typename memory_t, typename id_base_t>
    constexpr static auto value(const memory_t &, const id_base_t &) {
        return V;
    }
};

template<typename A>
struct Mem : R_value {
    template<typename memory_t, typename id_base_t>
    constexpr static auto value(const memory_t &mem, id_base_t &idbase) {
        is_r_value<A>();
        return mem[A::template value<memory_t, id_base_t>(mem, idbase)];
    }
};

template<Id_t ID>
struct Lea : R_value {
    template<typename memory_t, typename id_base_t>
    constexpr static auto value(const memory_t &, id_base_t &idbase) {
        size_t i = 0;
        for (; i < idbase.size(); ++i) {
            if (idbase[i] == ID)
                return i;
        }
        if (i == idbase.size())
            throw "Variable not declared!";

        return i; // Wymagana w celu poprawnosci budowy funkcji.
    }
};

template<Id_t S1, typename S2>
struct D : Instruction {
};

template<typename D, typename S>
struct Mov : Instruction {
};

template<typename D, typename S>
struct Add : Instruction {
};

template<typename D, typename S>
struct Sub : Instruction {
};

template<typename D>
using Inc = Add<D, Num<1>>;

template<typename D>
using Dec = Sub<D, Num<1>>;

template<typename D, typename S>
struct And : Instruction {
};

template<typename D, typename S>
struct Or : Instruction {
};

template<typename D>
struct Not : Instruction {
};

template<typename S1, typename S2>
struct Cmp : Instruction {
};

template<Id_t ID>
struct Label : Instruction {
};

template<Id_t ID>
struct Jmp : Instruction {
};

template<Id_t ID>
struct Jz : Instruction {
};

template<Id_t ID>
struct Js : Instruction {
};

template<typename... Instructions>
struct Program {
};

template<size_t MemorySize, typename T>
class Computer {
    using memory_t = std::array<T, MemorySize>;
    using id_base_t = std::array<Id_t, MemorySize>;

    template<typename Inst>
    constexpr static void is_instruction() {
        static_assert(std::is_base_of<Instruction, Inst>::value, "INTRUCTION WAS EXPECTED, BUT NOT PROVIDED");
    }


    template<typename P>
    struct ASMProgram;

    template<typename... Instructions>
    struct ASMProgram<Program<Instructions...>> {
        constexpr static void verify_program_and_set_variables(memory_t &mem, id_base_t &idbase) {
            Verificator<Instructions...>::verify_and_set(mem, idbase);
        }

        constexpr static auto evaluate(memory_t &mem, id_base_t &idbase) {
            bool ZF = false;
            bool SF = false;

            Evaluator<Instructions...>::template evaluate<Instructions...>(mem, idbase, ZF, SF);
            return mem;
        }
    };

    template<typename... Instructions>
    struct Verificator {
        constexpr static void verify_and_set(memory_t &, id_base_t &) {}
    };

    template<typename I, typename... Rest>
    struct Verificator<I, Rest...> {
        constexpr static void verify_and_set(memory_t &mem, id_base_t &idbase) {
            is_instruction<I>();
            Verificator<Rest...>::verify_and_set(mem, idbase);
        }
    };

    template<Id_t ID, auto Val, typename... Rest>
    struct Verificator<D<ID, Num<Val>>, Rest...> {
        constexpr static void verify_and_set(memory_t &mem, id_base_t &idbase) {
            size_t i = 0;

            for (; i < idbase.size(); ++i) {
                if (idbase[i] == ID) {
                    // W zadaniu nie zostało w pełni definiowane zachowanie
                    //      przy wielokrotnej inicjalizacji tej samej zmiennej.
                    break;
                }

                if (idbase[i] == 0) {
                    idbase[i] = ID;
                    mem[i] = Val;
                    break;
                }
            }
            if (i == idbase.size())
                throw "Memory limit exceeded!";

            Verificator<Rest...>::verify_and_set(mem, idbase);
        }

    };

    template<typename... Instructions>
    struct Evaluator {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &, id_base_t &, bool &, bool &) {}
    };

    template<typename I, typename... Rest>
    struct Evaluator<I, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &, id_base_t &, bool &, bool &) {
            // Niepoprawna instrukcja - poprawny program nie powinnien tu wejsc.
            throw "Wrong commands!";
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Mov<Mem<Arg1>, Arg2>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            is_r_value<Arg1>();
            is_r_value<Arg2>();

            mem[Arg1::value(mem, idbase)] = Arg2::value(mem, idbase);

            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Add<Mem<Arg1>, Arg2>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            is_r_value<Arg1>();
            is_r_value<Arg2>();

            auto addr = Arg1::template value<memory_t, id_base_t>(mem, idbase);

            mem[addr] += Arg2::template value<memory_t, id_base_t>(mem, idbase);

            ZF = (mem[addr] == 0);
            SF = (mem[addr] < 0);

            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Sub<Mem<Arg1>, Arg2>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            is_r_value<Arg1>();
            is_r_value<Arg2>();

            auto addr = Arg1::template value<memory_t, id_base_t>(mem, idbase);

            mem[addr] -= Arg2::template value<memory_t, id_base_t>(mem, idbase);

            ZF = (mem[addr] == 0);
            SF = (mem[addr] < 0);

            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<And<Mem<Arg1>, Arg2>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            is_r_value<Arg1>();
            is_r_value<Arg2>();

            constexpr auto addr = Arg1::template value<memory_t, id_base_t>(mem, idbase);

            mem[addr] &= Arg2::template value<memory_t, id_base_t>(mem, idbase);
            ZF = mem[addr] == 0;

            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Or<Mem<Arg1>, Arg2>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            is_r_value<Arg1>();
            is_r_value<Arg2>();

            constexpr auto addr = Arg1::template value<memory_t, id_base_t>(mem, idbase);

            mem[addr] |= Arg2::template value<memory_t, id_base_t>(mem, idbase);
            ZF = mem[addr] == 0;

            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<typename Arg1, typename... Rest>
    struct Evaluator<Not<Mem<Arg1>>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            is_r_value<Arg1>();

            constexpr auto addr = Arg1::template value<memory_t, id_base_t>(mem, idbase);

            mem[addr] = ~mem[addr];
            ZF = mem[addr] == 0;

            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Cmp<Arg1, Arg2>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            is_r_value<Arg1>();
            is_r_value<Arg2>();

            constexpr auto val = Arg1::template value<memory_t, id_base_t>(mem, idbase)
                                 - Arg2::template value<memory_t, id_base_t>(mem, idbase);

            ZF = val == 0;
            SF = val < 0;

            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<Id_t ID, auto Val, typename... Rest>
    struct Evaluator<D<ID, Num<Val>>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            // Pomijamy teraz to polecenie, bo zmienne były ustawiane na początku.
            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<Id_t ID, typename... Rest>
    struct Evaluator<Label<ID>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<Id_t ID, typename... Rest>
    struct Evaluator<Jz<ID>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            if (ZF)
                Evaluator<Jmp<ID>, Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
            else
                Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<Id_t ID, typename... Rest>
    struct Evaluator<Js<ID>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            if (SF)
                Evaluator<Jmp<ID>, Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
            else
                Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<Id_t ID, typename... Rest>
    struct EvaluatorJump {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &, id_base_t &, bool &, bool &) {}
    };

    template<Id_t ID, Id_t current_ID, typename... Rest>
    struct EvaluatorJump<ID, Label<current_ID>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            if (ID == current_ID)
                Evaluator<Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
            else
                EvaluatorJump<ID, Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<Id_t ID, typename V, typename... Rest>
    struct EvaluatorJump<ID, V, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            EvaluatorJump<ID, Rest...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

    template<Id_t ID>
    struct EvaluatorJump<ID> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &, id_base_t &, bool &, bool &) {
            throw "Wrong jump destination!";
        }
    };

    template<Id_t ID, typename... Rest>
    struct Evaluator<Jmp<ID>, Rest...> {
        template<typename... Programme>
        constexpr static void evaluate(memory_t &mem, id_base_t &idbase, bool &ZF, bool &SF) {
            EvaluatorJump<ID, Programme...>::template evaluate<Programme...>(mem, idbase, ZF, SF);
        }
    };

public:
    template<typename P>
    constexpr static auto boot() {
        memory_t memory{0};
        id_base_t idbase{0};

        ASMProgram<P>::verify_program_and_set_variables(memory, idbase);

        return ASMProgram<P>::evaluate(memory, idbase);
    }
};

#endif // COMPUTER_H