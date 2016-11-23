/**
 * @file      mips_isa.cpp
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *            Alexandro Baldassin (acasm information)
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:50:52 -0300
 * 
 * @brief     The ArchC i8051 functional model.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include  "mips_isa.H"
#include  "mips_isa_init.cpp"
#include  "mips_bhv_macros.H"
#include "expio.h"
#include "yapsc.h"

//If you want debug information for this model, uncomment next line
//#define DEBUG_MODEL
#include "ac_debug_model.H"

//!User defined macros to reference registers.
#define Ra 31
#define Sp 29

// 'using namespace' statement to allow access to all
// mips-specific datatypes
using namespace mips_parms;

static int processors_started = 0;
#define DEFAULT_STACK_SIZE (256 * 1024)

//!Generic instruction behavior method.
void ac_behavior( instruction )
{ 
   EXPIO_LOG_DBG("----- PC=%#x ----- %lld", (int) ac_pc, ac_instr_counter);
  //  EXPIO_LOG_DBG("----- PC=%#x NPC=%#x ----- %lld", (int) ac_pc, (int)npc, ac_instr_counter);
#ifndef NO_NEED_PC_UPDATE
  ac_pc = npc;
  npc = ac_pc + 4;
#endif 
};
 
//! Instruction Format behavior methods.
void ac_behavior( Type_R ){}
void ac_behavior( Type_I ){}
void ac_behavior( Type_J ){}
 
//!Behavior called before starting simulation
void ac_behavior(begin)
{
  EXPIO_LOG_DBG("@@@ begin behavior @@@");
  RB[0] = 0;
  npc = ac_pc + 4;

  // Is is not required by the architecture, but makes debug really easier
  for (int regNum = 0; regNum < 32; regNum ++)
    RB[regNum] = 0;
  hi = 0;
  lo = 0;

  EXPIO_LOG_DBG("Stack = %u", RB[29]);
  RB[29] =  AC_RAM_END - 1024;
  EXPIO_LOG_DBG("Stack = %u", RB[29]);
}

//!Behavior called after finishing simulation
void ac_behavior(end)
{
  EXPIO_LOG_DBG("@@@ end behavior @@@");
}


//!Instruction lb behavior method.
void ac_behavior( lb )
{
  char byte;
  EXPIO_LOG_DBG("lb r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  byte = DATA_PORT->read_byte(RB[rs]+ imm);
  RB[rt] = (ac_Sword)byte ;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction lbu behavior method.
void ac_behavior( lbu )
{
  unsigned char byte;
  EXPIO_LOG_DBG("lbu r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  byte = DATA_PORT->read_byte(RB[rs]+ imm);
  RB[rt] = byte ;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction lh behavior method.
void ac_behavior( lh )
{
  short int half;
  EXPIO_LOG_DBG("lh r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  half = DATA_PORT->read_half(RB[rs]+ imm);
  RB[rt] = (ac_Sword)half ;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction lhu behavior method.
void ac_behavior( lhu )
{
  unsigned short int  half;
  half = DATA_PORT->read_half(RB[rs]+ imm);
  RB[rt] = half ;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction lw behavior method.
void ac_behavior( lw )
{
  EXPIO_LOG_DBG("lw r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  RB[rt] = DATA_PORT->read(RB[rs]+ imm);
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction lwl behavior method.
void ac_behavior( lwl )
{
  EXPIO_LOG_DBG("lwl r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = DATA_PORT->read(addr & 0xFFFFFFFC);
  data <<= offset;
  data |= RB[rt] & ((1<<offset)-1);
  RB[rt] = data;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction lwr behavior method.
void ac_behavior( lwr )
{
  EXPIO_LOG_DBG("lwr r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = DATA_PORT->read(addr & 0xFFFFFFFC);
  data >>= offset;
  data |= RB[rt] & (0xFFFFFFFF << (32-offset));
  RB[rt] = data;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction sb behavior method.
void ac_behavior( sb )
{
  unsigned char byte;
  EXPIO_LOG_DBG("sb r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  byte = RB[rt] & 0xFF;
  DATA_PORT->write_byte(RB[rs] + imm, byte);
  EXPIO_LOG_DBG("Result = %#x", (int) byte);
};

//!Instruction sh behavior method.
void ac_behavior( sh )
{
  unsigned short int half;
  EXPIO_LOG_DBG("sh r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  half = RB[rt] & 0xFFFF;
  DATA_PORT->write_half(RB[rs] + imm, half);
  EXPIO_LOG_DBG("Result = %#x", (int) half);
};

//!Instruction sw behavior method.
void ac_behavior( sw )
{
  EXPIO_LOG_DBG("sw r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  DATA_PORT->write(RB[rs] + imm, RB[rt]);
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction swl behavior method.
void ac_behavior( swl )
{
  EXPIO_LOG_DBG("swl r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = RB[rt];
  data >>= offset;
  data |= DATA_PORT->read(addr & 0xFFFFFFFC) & (0xFFFFFFFF << (32-offset));
  DATA_PORT->write(addr & 0xFFFFFFFC, data);
  EXPIO_LOG_DBG("Result = %#x", data);
};

//!Instruction swr behavior method.
void ac_behavior( swr )
{
  EXPIO_LOG_DBG("swr r%d, %d(r%d)", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = RB[rt];
  data <<= offset;
  data |= DATA_PORT->read(addr & 0xFFFFFFFC) & ((1<<offset)-1);
  DATA_PORT->write(addr & 0xFFFFFFFC, data);
  EXPIO_LOG_DBG("Result = %#x", data);
};

//!Instruction addi behavior method.
void ac_behavior( addi )
{
  EXPIO_LOG_DBG("addi r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] + imm;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
  //Test overflow
  if ( ((RB[rs] & 0x80000000) == (imm & 0x80000000)) &&
       ((imm & 0x80000000) != (RB[rt] & 0x80000000)) ) {
    fprintf(stderr, "EXCEPTION(addi): integer overflow."); exit(EXIT_FAILURE);
  }
};

//!Instruction addiu behavior method.
void ac_behavior( addiu )
{
  EXPIO_LOG_DBG("addiu r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] + imm;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction slti behavior method.
void ac_behavior( slti )
{
  EXPIO_LOG_DBG("slti r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  // Set the RD if RS< IMM
  if( (ac_Sword) RB[rs] < (ac_Sword) imm )
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction sltiu behavior method.
void ac_behavior( sltiu )
{
  EXPIO_LOG_DBG("sltiu r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  // Set the RD if RS< IMM
  if( (ac_Uword) RB[rs] < (ac_Uword) imm )
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction andi behavior method.
void ac_behavior( andi )
{	
  EXPIO_LOG_DBG("andi r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] & (imm & 0xFFFF) ;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction ori behavior method.
void ac_behavior( ori )
{	
  EXPIO_LOG_DBG("ori r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] | (imm & 0xFFFF) ;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction xori behavior method.
void ac_behavior( xori )
{	
  EXPIO_LOG_DBG("xori r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] ^ (imm & 0xFFFF) ;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction lui behavior method.
void ac_behavior( lui )
{	
  EXPIO_LOG_DBG("lui r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  // Load a constant in the upper 16 bits of a register
  // To achieve the desired behaviour, the constant was shifted 16 bits left
  // and moved to the target register ( rt )
  RB[rt] = imm << 16;
  EXPIO_LOG_DBG("Result = %#x", RB[rt]);
};

//!Instruction add behavior method.
void ac_behavior( add )
{
  EXPIO_LOG_DBG("add r%d, r%d, r%d", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
  //Test overflow
  if ( ((RB[rs] & 0x80000000) == (RB[rd] & 0x80000000)) &&
       ((RB[rd] & 0x80000000) != (RB[rt] & 0x80000000)) ) {
    fprintf(stderr, "EXCEPTION(add): integer overflow."); exit(EXIT_FAILURE);
  }
};

//!Instruction addu behavior method.
void ac_behavior( addu )
{
  EXPIO_LOG_DBG("addu r%d, r%d, r%d", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  //cout << "  RS: " << (unsigned int)RB[rs] << " RT: " << (unsigned int)RB[rt] << endl;
  //cout << "  Result =  " <<  (unsigned int)RB[rd] <<endl;
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction sub behavior method.
void ac_behavior( sub )
{
  EXPIO_LOG_DBG("sub r%d, r%d, r%d", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
  //TODO: test integer overflow exception for sub
};

//!Instruction subu behavior method.
void ac_behavior( subu )
{
  EXPIO_LOG_DBG("subu r%d, r%d, r%d", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction slt behavior method.
void ac_behavior( slt )
{	
  EXPIO_LOG_DBG("slt r%d, r%d, r%d", rd, rs, rt);
  // Set the RD if RS< RT
  if( (ac_Sword) RB[rs] < (ac_Sword) RB[rt] )
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction sltu behavior method.
void ac_behavior( sltu )
{
  EXPIO_LOG_DBG("sltu r%d, r%d, r%d", rd, rs, rt);
  // Set the RD if RS < RT
  if( RB[rs] < RB[rt] )
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction instr_and behavior method.
void ac_behavior( instr_and )
{
  EXPIO_LOG_DBG("instr_and r%d, r%d, r%d", rd, rs, rt);
  RB[rd] = RB[rs] & RB[rt];
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction instr_or behavior method.
void ac_behavior( instr_or )
{
  EXPIO_LOG_DBG("instr_or r%d, r%d, r%d", rd, rs, rt);
  RB[rd] = RB[rs] | RB[rt];
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction instr_xor behavior method.
void ac_behavior( instr_xor )
{
  EXPIO_LOG_DBG("instr_xor r%d, r%d, r%d", rd, rs, rt);
  RB[rd] = RB[rs] ^ RB[rt];
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction instr_nor behavior method.
void ac_behavior( instr_nor )
{
  EXPIO_LOG_DBG("nor r%d, r%d, r%d", rd, rs, rt);
  RB[rd] = ~(RB[rs] | RB[rt]);
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction nop behavior method.
void ac_behavior( nop )
{  
  EXPIO_LOG_DBG("nop");
};

//!Instruction sll behavior method.
void ac_behavior( sll )
{  
  EXPIO_LOG_DBG("sll r%d, r%d, %d", rd, rs, shamt);
  RB[rd] = RB[rt] << shamt;
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction srl behavior method.
void ac_behavior( srl )
{
  EXPIO_LOG_DBG("srl r%d, r%d, %d", rd, rs, shamt);
  RB[rd] = RB[rt] >> shamt;
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction sra behavior method.
void ac_behavior( sra )
{
  EXPIO_LOG_DBG("sra r%d, r%d, %d", rd, rs, shamt);
  RB[rd] = (ac_Sword) RB[rt] >> shamt;
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction sllv behavior method.
void ac_behavior( sllv )
{
  EXPIO_LOG_DBG("sllv r%d, r%d, r%d", rd, rt, rs);
  RB[rd] = RB[rt] << (RB[rs] & 0x1F);
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction srlv behavior method.
void ac_behavior( srlv )
{
  EXPIO_LOG_DBG("srlv r%d, r%d, r%d", rd, rt, rs);
  RB[rd] = RB[rt] >> (RB[rs] & 0x1F);
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction srav behavior method.
void ac_behavior( srav )
{
  EXPIO_LOG_DBG("srav r%d, r%d, r%d", rd, rt, rs);
  RB[rd] = (ac_Sword) RB[rt] >> (RB[rs] & 0x1F);
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction mult behavior method.
void ac_behavior( mult )
{
  EXPIO_LOG_DBG("mult r%d, r%d", rs, rt);

  long long result;
  int half_result;

  result = (ac_Sword) RB[rs];
  result *= (ac_Sword) RB[rt];

  half_result = (result & 0xFFFFFFFF);
  // Register LO receives 32 less significant bits
  lo = half_result;

  half_result = ((result >> 32) & 0xFFFFFFFF);
  // Register HI receives 32 most significant bits
  hi = half_result ;

  EXPIO_LOG_DBG("Result = %#llx", result);
};

//!Instruction multu behavior method.
void ac_behavior( multu )
{
  EXPIO_LOG_DBG("multu r%d, r%d", rs, rt);

  unsigned long long result;
  unsigned int half_result;

  result  = RB[rs];
  result *= RB[rt];

  half_result = (result & 0xFFFFFFFF);
  // Register LO receives 32 less significant bits
  lo = half_result;

  half_result = ((result>>32) & 0xFFFFFFFF);
  // Register HI receives 32 most significant bits
  hi = half_result ;

  EXPIO_LOG_DBG("Result = %#llx", result);
};

//!Instruction div behavior method.
void ac_behavior( div )
{
  EXPIO_LOG_DBG("div r%d, r%d", rs, rt);
  // Register LO receives quotient
  lo = (ac_Sword) RB[rs] / (ac_Sword) RB[rt];
  // Register HI receives remainder
  hi = (ac_Sword) RB[rs] % (ac_Sword) RB[rt];
};

//!Instruction divu behavior method.
void ac_behavior( divu )
{
  EXPIO_LOG_DBG("divu r%d, r%d", rs, rt);
  // Register LO receives quotient
  lo = RB[rs] / RB[rt];
  // Register HI receives remainder
  hi = RB[rs] % RB[rt];
};

//!Instruction mfhi behavior method.
void ac_behavior( mfhi )
{
  EXPIO_LOG_DBG("mfhi r%d", rd);
  RB[rd] = hi;
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction mthi behavior method.
void ac_behavior( mthi )
{
  EXPIO_LOG_DBG("mthi r%d", rs);
  hi = RB[rs];
  EXPIO_LOG_DBG("Result = %#x", (unsigned int) hi);
};

//!Instruction mflo behavior method.
void ac_behavior( mflo )
{
  EXPIO_LOG_DBG("mflo r%d", rd);
  RB[rd] = lo;
  EXPIO_LOG_DBG("Result = %#x", RB[rd]);
};

//!Instruction mtlo behavior method.
void ac_behavior( mtlo )
{
  EXPIO_LOG_DBG("mtlo r%d", rs);
  lo = RB[rs];
  EXPIO_LOG_DBG("Result = %#x", (unsigned int) lo);
};

//!Instruction j behavior method.
void ac_behavior( j )
{
  EXPIO_LOG_DBG("j %d", addr);
  addr = addr << 2;
#ifndef NO_NEED_PC_UPDATE
  npc =  (ac_pc & 0xF0000000) | addr;
#endif 
  EXPIO_LOG_DBG("Target = %#x", (ac_pc & 0xF0000000) | addr );
};

//!Instruction jal behavior method.
void ac_behavior( jal )
{
  EXPIO_LOG_DBG("jal %d", addr);
  // Save the value of PC + 8 (return address) in $ra ($31) and
  // jump to the address given by PC(31...28)||(addr<<2)
  // It must also flush the instructions that were loaded into the pipeline
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
	
  addr = addr << 2;
#ifndef NO_NEED_PC_UPDATE
  npc = (ac_pc & 0xF0000000) | addr;
#endif 
	
  EXPIO_LOG_DBG("Target = %#x", (ac_pc & 0xF0000000) | addr );
  EXPIO_LOG_DBG("Return = %#x", ac_pc+4);
};

//!Instruction jr behavior method.
void ac_behavior( jr )
{
  EXPIO_LOG_DBG("jr r%d", rs);
  // Jump to the address stored on the register reg[RS]
  // It must also flush the instructions that were loaded into the pipeline
#ifndef NO_NEED_PC_UPDATE
  npc = RB[rs], (void) 1;
#endif 
  EXPIO_LOG_DBG("Target = %#x", RB[rs]);
};

//!Instruction jalr behavior method.
void ac_behavior( jalr )
{
  EXPIO_LOG_DBG("jalr r%d, r%d", rd, rs);
  // Save the value of PC + 8(return address) in rd and
  // jump to the address given by [rs]

#ifndef NO_NEED_PC_UPDATE
  npc = RB[rs], (void) 1;
#endif 
  EXPIO_LOG_DBG("Target = %#x", RB[rs]);

  if( rd == 0 )  //If rd is not defined use default
    rd = Ra;
  RB[rd] = ac_pc+4;
  EXPIO_LOG_DBG("Return = %#x", ac_pc+4);
};

//!Instruction beq behavior method.
void ac_behavior( beq )
{
  EXPIO_LOG_DBG("beq r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  if( RB[rs] == RB[rt] ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    EXPIO_LOG_DBG("Taken to %#x", ac_pc + (imm<<2));
  }	
};

//!Instruction bne behavior method.
void ac_behavior( bne )
{	
  EXPIO_LOG_DBG("bne r%d, r%d, %d", rt, rs, imm & 0xFFFF);
  if( RB[rs] != RB[rt] ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    EXPIO_LOG_DBG("Taken to %#x", ac_pc + (imm<<2));
  }	
};

//!Instruction blez behavior method.
void ac_behavior( blez )
{
  EXPIO_LOG_DBG("blez r%d, %d", rs, imm & 0xFFFF);
  if( (RB[rs] == 0 ) || (RB[rs]&0x80000000 ) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2), (void) 1;
#endif 
    EXPIO_LOG_DBG("Taken to %#x", ac_pc + (imm<<2));
  }	
};

//!Instruction bgtz behavior method.
void ac_behavior( bgtz )
{
  EXPIO_LOG_DBG("bgtz r%d, %d", rs, imm & 0xFFFF);
  if( !(RB[rs] & 0x80000000) && (RB[rs]!=0) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    EXPIO_LOG_DBG("Taken to %#x", ac_pc + (imm<<2));
  }	
};

//!Instruction bltz behavior method.
void ac_behavior( bltz )
{
  EXPIO_LOG_DBG("bltz r%d, %d", rs, imm & 0xFFFF);
  if( RB[rs] & 0x80000000 ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    EXPIO_LOG_DBG("Taken to %#x", ac_pc + (imm<<2));
  }	
};

//!Instruction bgez behavior method.
void ac_behavior( bgez )
{
  EXPIO_LOG_DBG("bgez r%d, %d", rs, imm & 0xFFFF);
  if( !(RB[rs] & 0x80000000) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    EXPIO_LOG_DBG("Taken to %#x", ac_pc + (imm<<2));
  }	
};

//!Instruction bltzal behavior method.
void ac_behavior( bltzal )
{
  EXPIO_LOG_DBG("bltzal r%d, %d", rs, imm & 0xFFFF);
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
  if( RB[rs] & 0x80000000 ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    EXPIO_LOG_DBG("Taken to %#x", ac_pc + (imm<<2));
  }	
  EXPIO_LOG_DBG("Return = %#x", ac_pc+4);
};

//!Instruction bgezal behavior method.
void ac_behavior( bgezal )
{
  EXPIO_LOG_DBG("bgezal r%d, %d", rs, imm & 0xFFFF);
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
  if( !(RB[rs] & 0x80000000) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    EXPIO_LOG_DBG("Taken to %#x", ac_pc + (imm<<2));
  }	
  EXPIO_LOG_DBG("Return = %#x", ac_pc+4);
};

//!Instruction sys_call behavior method.
void ac_behavior( sys_call )
{
  EXPIO_LOG_DBG("syscall");
  stop();
}

//!Instruction instr_break behavior method.
void ac_behavior( instr_break )
{
  fprintf(stderr, "instr_break behavior not implemented."); 
  exit(EXIT_FAILURE);
}
