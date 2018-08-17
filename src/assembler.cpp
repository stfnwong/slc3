/* ASSEMBLER
 * Assembler object
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include "assembler.hpp"
// TODO ; also need LC3 constants here ...
#include "lc3.hpp"


/*
 * AsmLogEntry
 * Logs status of a single line during the assembly process
 */
AsmLogEntry::AsmLogEntry()
{
    this->init();
}

AsmLogEntry::~AsmLogEntry() {} 

AsmLogEntry::AsmLogEntry(const AsmLogEntry& that)
{
    this->line  = that.line;
    this->addr  = that.addr;
    this->error = that.error;
    this->msg   = that.msg;
}

void AsmLogEntry::init(void)
{
    this->line  = 0;
    this->addr  = 0;
    this->error = false;
    this->msg   = "\0";
}

std::string AsmLogEntry::toString(void) const
{
    std::ostringstream oss;

    oss << "Line   Addr   Status   Msg" << std::endl;
    oss << std::dec << std::setw(4) << std::setfill(' ') << std::right << this->line;
    oss << " 0x" << std::hex << std::setw(4) << std::setfill('0') << std::right << this->addr;
    oss << "  ";
    if(this->error)
        oss << std::right << "[ERROR]";
    else
        oss << std::right << "[ OK  ]";
    oss << "  <" << this->msg << ">";
    oss << std::endl;

    return oss.str();
}

/* 
 * AsmLog
 * Assembly log object
 */
AsmLog::AsmLog() {}
AsmLog::~AsmLog() {} 
AsmLog::AsmLog(const AsmLog& that)
{
    this->log = that.log;
}

void AsmLog::add(const AsmLogEntry& e)
{
    this->log.push_back(e);
}

AsmLogEntry AsmLog::get(const unsigned int idx) const
{
    return this->log[idx % this->log.size()];
}

// Get a single entry as a string 
std::string AsmLog::getString(const unsigned int idx) const
{
    return this->log[idx % this->log.size()].toString();
}

// Get the entire log as a string
std::string AsmLog::getString(void) const
{
    std::ostringstream oss;
    for(unsigned int idx = 0; idx < this->log.size(); ++idx)
        oss << this->log[idx].toString();

    return oss.str();
}

/*
 * Asssembler
 *
 * Assembles an LC3 binary from a SourceInfo structure 
 */
Assembler::Assembler(const SourceInfo& si)
{
    this->src_info      = si;
    this->num_err       = 0;
    this->verbose       = false;
    this->cont_on_error = false;
}

Assembler::~Assembler() {}

inline uint16_t Assembler::asm_arg1(const uint16_t arg)
{
    return 0x0000 | (arg << 9);
}
inline uint16_t Assembler::asm_arg2(const uint16_t arg)
{
    return 0x0000 | (arg << 6);
}
inline uint16_t Assembler::asm_arg3(const uint16_t arg)
{
    return 0x0000 | (arg);
}
inline uint16_t Assembler::asm_imm5(const uint16_t arg)
{
    return 0x0000 | (arg & 0x001F);
}
inline uint16_t Assembler::asm_of6(const uint16_t arg)
{
    return 0x0000 | (arg & 0x001F);
}
inline uint8_t Assembler::asm_in8(const uint16_t arg)
{
    return 0x0000 | (arg & 0x00FF);
}
inline uint16_t Assembler::asm_pc9(const uint16_t arg)
{
    return 0x0000 | (arg & 0x01FF);
}
inline uint16_t Assembler::asm_pc11(const uint16_t arg)
{
    return 0x0000 | (arg & 0x07FF);
}

/*
 * asm_add()
 * Assemble ADD instruction
 */
void Assembler::asm_add(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling ADD" << std::endl;
    }
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.arg2));
    if(line.is_imm)
    {
        instr.ins = (instr.ins | (1 << 5));
        instr.ins = (instr.ins | this->asm_imm5(line.imm));
    }
    else
        instr.ins = (instr.ins | this->asm_arg3(line.arg3));
    instr.ins  = (instr.ins | (line.opcode.opcode << 12));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_and()
 * Assemble AND instruction
 */
void Assembler::asm_and(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling AND" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins| this->asm_arg1(line.arg1));
    instr.ins = (instr.ins| this->asm_arg2(line.arg2));
    instr.ins = (instr.ins | this->asm_arg3(line.arg3));
    if(line.is_imm)
    {
        instr.ins = (instr.ins | (1 << 5));
        instr.ins = (instr.ins | this->asm_imm5(line.imm));
    }
    else
        instr.ins = (instr.ins | this->asm_arg3(line.arg3));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_br()
 * Assemble BR instruction
 */
void Assembler::asm_br(const LineInfo& line)
{
    Instr instr;
    int16_t offset = 0;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling BR" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    offset = (line.imm == 0) ? 0 : line.imm - (line.addr + 1);
    if(offset < -LC3_OFFSET_MAX || offset > LC3_OFFSET_MAX)
    {
        this->cur_log_entry.error = true;
        this->cur_log_entry.msg = "BR offset too large (" + std::to_string(offset) + ")";
        return;
    }
    // flags 
    if(line.flags & LC3_FLAG_N)
        instr.ins = (instr.ins | 0x0800);
    if(line.flags & LC3_FLAG_Z)
        instr.ins = (instr.ins | 0x0400);
    if(line.flags & LC3_FLAG_P)
        instr.ins = (instr.ins | 0x0200);
    // offset 
    instr.ins = (instr.ins | (offset & 0x01FF));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_jmp()
 * Assemble JMP instruction
 */
void Assembler::asm_jmp(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling JMP" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg2(instr.ins));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_jsr()
 * Assemble JSR instruction
 */
void Assembler::asm_jsr(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    // Is this JSR or JSRR?
    if(line.arg1 > 0)       // JSR 
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (src line " 
                << std::dec << line.line_num << ") assembling JSR" << std::endl;
        }
        instr.ins = (instr.ins | this->asm_pc11(instr.ins));
    }
    else                    // JSRR
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (src line " 
                << std::dec << line.line_num << ") assembling JSRR" << std::endl;
        }
        instr.ins = (instr.ins | this->asm_arg2(instr.ins));
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.adr = line.addr;

    this->program.add(instr);

}

/*
 * asm_lea()
 * Assemble LEA instruction
 */
void Assembler::asm_lea(const LineInfo& line)
{
    Instr instr;
    int16_t offset = 0;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling LEA" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    offset    = line.imm - (line.addr + 1);
    if(offset < -LC3_OFFSET_MAX || offset > LC3_OFFSET_MAX)
    {
        this->cur_log_entry.error = true;
        this->cur_log_entry.addr  = line.addr;
        this->cur_log_entry.msg   = "LEA offset too large (" + std::to_string(offset) + ")";
        return;
    }
    instr.ins = (instr.ins | (offset & 0x01FF));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_ld()
 * Assemble LD instruction
 */
void Assembler::asm_ld(const LineInfo& line)
{
    Instr instr;
    int16_t offset = 0;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling LD" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    offset = (line.imm == 0) ? 0 : line.imm - (line.addr + 1);
    if(offset < -LC3_OFFSET_MAX || offset > LC3_OFFSET_MAX)
    {
        this->cur_log_entry.error = true;
        this->cur_log_entry.msg   = "LD offset too large (" + std::to_string(offset) + ")";
        return;
    }
    instr.ins = (instr.ins | (offset & 0x01FF));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_ldr()
 * Assemble LDR instruction
 */
void Assembler::asm_ldr(const LineInfo& line)
{
    Instr instr;
    int16_t offset = 0;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling LDR" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.arg2)); 
    offset    = (line.imm == 0) ? 0 : line.imm - (line.addr + 1);
    if(offset < -LC3_OFFSET_MAX || offset > LC3_OFFSET_MAX)
    {
        this->cur_log_entry.error = true;
        this->cur_log_entry.msg   = "LDR offset too large (" + std::to_string(offset) + ")";
        return;
    }
    instr.ins = (instr.ins | (offset & 0x01FF));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_not()
 * Assemble NOT instruction
 */
void Assembler::asm_not(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling NOT" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.imm));
    // NOT Has lower 6 bits set to 1
    instr.ins = (instr.ins | 0x001F);
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_st()
 * Assemble ST instruction
 */
void Assembler::asm_st(const LineInfo& line)
{
    Instr instr;
    int16_t offset = 0;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling ST" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    offset = (line.imm == 0) ? 0 : line.imm - (line.addr + 1);
    if(offset < -LC3_OFFSET_MAX || offset > LC3_OFFSET_MAX)
    {
        this->cur_log_entry.error = true;
        this->cur_log_entry.addr  = line.addr;
        this->cur_log_entry.msg   = "ST offset too large";
        return;
    }
    instr.ins = (instr.ins | (offset & 0x01FF));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_str()
 * Assemble STR instruction
 */
void Assembler::asm_str(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling STR" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.arg2));
    instr.ins = (instr.ins | (this->asm_of6(line.imm) & 0x003F));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_sti()
 * Assemble STI instruction
 */
void Assembler::asm_sti(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling STI" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.arg2));
    instr.ins = (instr.ins | (this->asm_of6(line.imm) & 0x003F));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * asm_trap()
 * Assemble TRAP instruction
 */
void Assembler::asm_trap(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << std::dec << line.line_num << ") assembling TRAP" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_in8(line.imm));
    instr.adr = line.addr;

    this->program.add(instr);
}

/*
 * dir_blkw()
 * Assemble the BLKW directive
 */
void Assembler::dir_blkw(const LineInfo& line)
{
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " <<
            std::dec << line.line_num << ") assembling .BLKW" << std::endl;
    }

    unsigned int addr;
    for(addr = line.addr; addr < line.addr + line.imm; addr++)
        this->program.writeMem(addr, 0x0000);
}

/*
 * dir_fill()
 * Assemble the FILL directive
 */
void Assembler::dir_fill(const LineInfo& line)
{
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " <<
            std::dec << line.line_num << ") assembling .FILL" << std::endl;
    }
    this->program.writeMem(line.addr, line.imm);
}

/*
 * dir_orig()
 * Assemble the ORIG directive
 */
void Assembler::dir_orig(const LineInfo& line)
{
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " <<
            std::dec << line.line_num << ") assembling .ORIG" << std::endl;
    }
    //this->start_addr = line.imm;
}

/*
 * dir_stringz()
 * Assemble the STRINGZ directive
 */
void Assembler::dir_stringz(const LineInfo& line)
{
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " <<
            std::dec << line.line_num << ") assembling .STRINGZ" << std::endl;
    }
    
    unsigned int addr, n;
    n = 0;
    for(addr = line.addr; addr < (line.addr + line.symbol.length()); ++addr)
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] writing symbol " 
                << std::setw(2) << line.symbol[n] 
                << " to address 0x" << std::hex << std::setw(4) 
                << std::setfill('0') << addr << std::endl;
        }
        this->program.writeMem(addr, line.symbol[n]);
        n++;
    }
}

/*
 * assemble()
 * Assemble the program into a memory image 
 */
void Assembler::assemble(void)
{
    LineInfo cur_line;
    unsigned int num_lines, idx;

    this->num_err = 0;
    num_lines = this->src_info.getNumLines();
    for(idx = 0; idx < num_lines; idx++)
    {
        // Init the log info for this line
        this->cur_log_entry.init();
        // Get the current line
        cur_line = this->src_info.get(idx);
        this->cur_log_entry.line = cur_line.line_num;
        this->cur_log_entry.addr = cur_line.addr;
        if(cur_line.error)
        {
            this->num_err++;
            this->cur_log_entry.error = true;
            this->cur_log_entry.msg = "Lexer error on line " + std::to_string(cur_line.line_num);
            goto LINE_END;
        }

        // Handle directives 
        if(cur_line.is_directive)
        {
            if(cur_line.opcode.mnemonic == ".BLKW")
                this->dir_blkw(cur_line);
            else if(cur_line.opcode.mnemonic == ".FILL")
                this->dir_fill(cur_line);
            else if(cur_line.opcode.mnemonic == ".ORIG")
                this->dir_orig(cur_line);
            else if(cur_line.opcode.mnemonic == ".STRINGZ")
                this->dir_stringz(cur_line);
            goto LINE_END;
        }

        // Handle opcodes 
        switch(cur_line.opcode.opcode)
        {
            case LC3_ADD:
                this->asm_add(cur_line);
                break;
            case LC3_AND:
                this->asm_and(cur_line);
                break;
            case LC3_BR:
                this->asm_br(cur_line);
                break;
            case LC3_JSR:
                this->asm_jsr(cur_line);
                break;
            case LC3_LEA:
                this->asm_lea(cur_line);
                break;
            case LC3_LD:
                this->asm_ld(cur_line);
                break;
            case LC3_LDR:
                this->asm_ldr(cur_line);
                break;
            case LC3_STR:
                this->asm_str(cur_line);
                break;
            case LC3_TRAP:
                this->asm_trap(cur_line);
                break;
            default:
                std::ostringstream oss;
                oss << "Invalid opcode 0x" << std::hex << std::setw(2)
                    << cur_line.opcode.opcode << " (mnemonic " 
                    << std::uppercase << cur_line.opcode.mnemonic << ")";
                this->cur_log_entry.msg = oss.str();
                this->cur_log_entry.error = true;
                if(this->verbose)
                {
                    std::cout << "[" << __FUNCTION__ << "] " << 
                        this->cur_log_entry.msg << std::endl;
                }
                break;
        }
LINE_END:
        this->log.add(this->cur_log_entry);
        if(this->cur_log_entry.error)
        {
            this->num_err++;
            std::cerr << this->cur_log_entry.msg << std::endl;
            if(!this->cont_on_error)
                return;
        }
    }
}

unsigned int Assembler::getNumErr(void) const
{
    return this->num_err;
}

Program Assembler::getProgram(void) const
{
    return this->program;
}

std::vector<Instr> Assembler::getInstrs(void) const
{
    return this->program.getInstr();
}

void Assembler::setVerbose(const bool v)
{
    this->verbose = v;
}

bool Assembler::getVerbose(void) const
{
    return this->verbose;
}

void Assembler::setContOnError(const bool c)
{
    this->cont_on_error = c;
}

int Assembler::write(const std::string& filename)
{
    return this->program.save(filename);
}

// Log handling functions
std::string Assembler::getLog(void) const
{
    return this->log.getString();
}

//AsmLog Assembler::getLog(void) const
//{
//    return this->log;
//}
