#ifndef REPORTER_H
#define REPORTER_H

#include "common/memory_ownership.h"
#include "machine/machine.h"

#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QVector>

using machine::Address;

class Reporter : public QObject {
    Q_OBJECT

public:
    Reporter(QCoreApplication *app, machine::Machine *machine);

    void regs() { e_regs = true; }; // Report status of registers
    void cache_stats() { e_cache_stats = true; };
    void cycles() { e_cycles = true; };

    enum FailReason {
        FR_NONE = 0,
        FR_UNSUPPORTED_INSTR = (1 << 0),
    };
    static const enum FailReason FailAny = FR_UNSUPPORTED_INSTR;
    void expect_fail(enum FailReason reason) { e_fail = (FailReason)(e_fail | reason); };

    struct DumpRange {
        Address start;
        size_t len;
        /** Path to file, where this range will be dumped. */
        QString path_to_write;
    };
    void add_dump_range(Address start, size_t len, const QString &path_to_write);

private slots:
    void machine_exit();
    void machine_trap(machine::SimulatorException &e);
    void machine_exception_reached();

private:
    BORROWED QCoreApplication *const app;
    BORROWED machine::Machine *const machine;
    QVector<DumpRange> dump_ranges;

    bool e_regs = false;
    bool e_cache_stats = false;
    bool e_cycles = false;
    FailReason e_fail = FR_NONE;

    void report();
    void report_regs() const;
    void report_cache() const;
    void report_range(const DumpRange &range) const;
    void report_cop0reg(machine::Cop0State::Cop0Registers i) const;
    void report_gp_reg(unsigned int i) const;
};

#endif // REPORTER_H
