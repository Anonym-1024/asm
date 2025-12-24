
const int directives_count = 6;
const char directives[6][10] = {
    ".byte",
    ".bytes",
    ".rep",
    ".endrep",
    ".ascii",
    ".start"
};

const int instructions_count = 56;
const char instructions[56][10] = {
    "mov",
	"movs",
	"mvn",
	"mvns",
	"srw",
	"srr",
	"ldr",
	"ldro",
	"ldri",
	"str",
	"stro",
	"stri",
	"add",
	"adds",
	"addc",
	"addcs",
	"sub",
	"subs",
	"subc",
	"subcs",
	"and",
	"ands",
	"or",
	"ors",
	"eor",
	"eors",
	"lsl",
	"lsls",
	"lsr",
	"lsrs",
	"asr",
	"asrs",
	"cls",
	"csls",
	"csr",
	"csrs",
	"cmn",
	"addcd",
	"cmp",
	"subcd",
	"andd",
	"ord",
	"eord",
	"lsld",
	"lsrd",
	"asrd",
	"csld",
	"csrd",
	"ba",
	"bal",
	"br",
	"brl",
	"ptr",
	"ptw",
	"ptsr",
	"svc"
};

const int macros_count = 3;
const char macros[3][10] = {
    "!b",
    "!bl",
    "!mov"
};

const int condition_codes_count = 19;
const char condition_codes[19][10] = {
    "al",
    "eq",
    "zs",
    "mi",
    "vs",
    "su",
    "cc",
    "gu",
    "ss",
    "gs",
    "ne",
    "zc",
    "pl",
    "vc",
    "geu",
    "cs",
    "seu",
    "GES",
    "SES"
    
};

const int registers_count = 16;
const char registers[16][10] = {
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7",
    "r8",
    "r9",
    "r10",
    "r11",
    "r12",
    "r13",
    "r14",
    "r15",
};

const int system_registers_count = 6;
const char system_registers[6][10] = {
    "pc_b0",
    "pc_b1",
    "psr",
    "intr",
    "pdbr_b0",
    "pdbr_b1"
};

const int ports_count = 8;
const char ports[8][10] = {
    "p0",
    "p1",
    "p2",
    "p3",
    "p4",
    "p5",
    "p6",
    "p7",
};

const int address_registers_count = 16;
const char address_registers[16][10] = {
    "ar0",
    "ar1",
    "ar2",
    "ar3",
    "ar4",
    "ar5",
    "ar6",
    "ar7",
    "ar8",
    "ar9",
    "ar10",
    "ar11",
    "ar12",
    "ar13",
    "ar14",
    "ar15",
};