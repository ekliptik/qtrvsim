#ifndef CORE_H
#define CORE_H

#include "common/memory_ownership.h"
#include "cop0state.h"
#include "core/core_state.h"
#include "instruction.h"
#include "machineconfig.h"
#include "memory/address.h"
#include "memory/frontend_memory.h"
#include "pipeline.h"
#include "predictor.h"
#include "register_value.h"
#include "registers.h"
#include "simulator_exception.h"

#include <QObject>

namespace machine {

class Core;

class ExceptionHandler : public QObject {
    Q_OBJECT
public:
    virtual bool handle_exception(
        Core *core,
        Registers *regs,
        ExceptionCause excause,
        Address inst_addr,
        Address next_addr,
        Address jump_branch_pc,
        Address mem_ref_addr)
        = 0;
};

class StopExceptionHandler : public ExceptionHandler {
    Q_OBJECT
public:
    bool handle_exception(
        Core *core,
        Registers *regs,
        ExceptionCause excause,
        Address inst_addr,
        Address next_addr,
        Address jump_branch_pc,
        Address mem_ref_addr) override;
};

class Core : public QObject {
    Q_OBJECT
public:
    Core(
        Registers *regs,
        Predictor *predictor,
        FrontendMemory *mem_program,
        FrontendMemory *mem_data,
        unsigned int min_cache_row_size = 1,
        Cop0State *cop0state = nullptr,
        Xlen xlen = Xlen::_32);

    void step(bool skip_break = false); // Do single step
    void reset();                       // Reset core (only core, memory and registers has to be
                                        // reseted separately)

    unsigned get_cycle_count() const; // Returns number of executed
                                      // get_cycle_count
    unsigned get_stall_count() const; // Returns number of stall get_cycle_count

    Registers *get_regs();
    Cop0State *get_cop0state();
    Predictor *get_predictor();
    FrontendMemory *get_mem_data();
    FrontendMemory *get_mem_program();
    void register_exception_handler(ExceptionCause excause, ExceptionHandler *exhandler);
    void insert_hwbreak(Address address);
    void remove_hwbreak(Address address);
    bool is_hwbreak(Address address);
    void set_stop_on_exception(enum ExceptionCause excause, bool value);
    bool get_stop_on_exception(enum ExceptionCause excause) const;
    void set_step_over_exception(enum ExceptionCause excause, bool value);
    bool get_step_over_exception(enum ExceptionCause excause) const;

    void set_c0_userlocal(uint32_t address);

public:
    CoreState state {};

    /**
     * Shortcuts to interstage registers
     * Interstage registers are stored in the core state struct in 2 copies. One (result) is the
     * state after combinatorial logic of each stage has been applied. This is used to visualize
     * the internal state of a stage. The should be modified ONLY by the stage logic functions. The
     * other (final) is the one actually written to HW interstage register. Any operation within
     * core should happen on the final registers.
     *
     * The bellow references provide shortcuts to the final interstage registers.
     */

    /** Reference to interstage register IF/ID inside core state. */
    FetchInterstage &if_id;
    /** Reference to interstage register ID/EX inside core state. */
    DecodeInterstage &id_ex;
    /** Reference to interstage register EX/MEM inside core state. */
    ExecuteInterstage &ex_mem;
    /** Reference to interstage register MEM/WB inside core state. */
    MemoryInterstage &mem_wb;

signals:
    void fetch_inst_addr_value(machine::Address);
    void decode_inst_addr_value(machine::Address);
    void execute_inst_addr_value(machine::Address);
    void memory_inst_addr_value(machine::Address);
    void writeback_inst_addr_value(machine::Address);
    void stop_on_exception_reached();
    void step_done() const;

protected:
    virtual void do_step(bool skip_break = false) = 0;
    virtual void do_reset() = 0;

    bool handle_exception(
        Core *core,
        Registers *regs,
        ExceptionCause excause,
        Instruction inst,
        Address inst_addr,
        Address next_addr,
        Address jump_branch_pc,
        Address mem_ref_addr);

    /**
     * Abstracts XLEN from code flow. XLEN core will obtain XLEN value from register value.
     * The value will be zero extended to u64.
     */
    uint64_t get_xlen_from_reg(RegisterValue reg) const;

    const Xlen xlen;
    BORROWED Registers *const regs;
    BORROWED Cop0State *const cop0state;
    BORROWED Predictor *const predictor;
    BORROWED FrontendMemory *const mem_data, *const mem_program;
    QMap<ExceptionCause, OWNED ExceptionHandler *> ex_handlers;
    Box<ExceptionHandler> ex_default_handler;

    FetchState fetch(bool skip_break = false);
    DecodeState decode(const FetchInterstage &);
    ExecuteState execute(const DecodeInterstage &);
    MemoryState memory(const ExecuteInterstage &);
    WritebackState writeback(const MemoryInterstage &);

    /**
     * This function computes the PC value, the next executed instruction should have. The word
     * `computed` is used in contrast with predicted value by the branch predictor.
     * Under normal circumstances, the computed PC value is the same as the PC on instruction in
     * previous stage. If not, mis-prediction occurred and has to be resolved.
     */
    Address compute_next_pc(const ExecuteInterstage &exec) const;
    void flush();

    enum ExceptionCause memory_special(
        enum AccessControl memctl,
        int mode,
        bool memread,
        bool memwrite,
        RegisterValue &towrite_val,
        RegisterValue rt_value,
        Address mem_addr);
};

class CoreSingle : public Core {
public:
    CoreSingle(
        Registers *regs,
        Predictor *predictor,
        FrontendMemory *mem_program,
        FrontendMemory *mem_data,
        unsigned int min_cache_row_size,
        Cop0State *cop0state,
        Xlen xlen);

protected:
    void do_step(bool skip_break = false) override;
    void do_reset() override;

private:
    Address prev_inst_addr {};
};

class CorePipelined : public Core {
public:
    CorePipelined(
        Registers *regs,
        Predictor *predictor,
        FrontendMemory *mem_program,
        FrontendMemory *mem_data,
        enum MachineConfig::HazardUnit hazard_unit,
        unsigned int min_cache_row_size,
        Cop0State *cop0state,
        Xlen xlen);

protected:
    void do_step(bool skip_break = false) override;
    void do_reset() override;

private:
    MachineConfig::HazardUnit hazard_unit;

    bool handle_data_hazards();
    void process_exception(Address jump_branch_pc);
    bool detect_mispredicted_jump() const;
    void handle_pc();
    void handle_stall(const FetchInterstage &saved_if_id);
};

std::tuple<bool, Address> predict(Instruction inst, Address addr);

} // namespace machine

#endif // CORE_H
