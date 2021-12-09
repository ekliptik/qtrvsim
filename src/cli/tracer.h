#ifndef TRACER_H
#define TRACER_H

#include "machine/instruction.h"
#include "machine/machine.h"
#include "machine/memory/address.h"
#include "machine/registers.h"

#include <QObject>

class Tracer final : public QObject {
    Q_OBJECT
public:
    explicit Tracer(machine::Machine *machine);

private slots:
    void step_output();

private:
    const machine::CoreState &core_state;

public:
    std::array<bool, machine::REGISTER_COUNT> regs_to_trace = {};
    bool trace_fetch = false, trace_decode = false, trace_execute = false, trace_memory = false,
         trace_writeback = false, trace_pc = false, trace_regs_gp = false;
};

#endif // TRACER_H
