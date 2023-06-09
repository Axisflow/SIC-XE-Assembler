#include "assembler.hpp"

const unordered_map<string, unsigned char> SICXEAssembler::opcode_table = {
    {"ADD", 0x18}, {"ADDF", 0x58}, {"ADDR", 0x90}, {"AND", 0x40}, {"CLEAR", 0xB4}, {"COMP", 0x28}, {"COMPF", 0x88}, {"COMPR", 0xA0}, {"DIV", 0x24}, {"DIVF", 0x64}, {"DIVR", 0x9C}, {"FIX", 0xC4}, {"FLOAT", 0xC0}, {"HIO", 0xF4}, {"J", 0x3C}, {"JEQ", 0x30}, {"JGT", 0x34}, {"JLT", 0x38}, {"JSUB", 0x48}, {"LDA", 0x00}, {"LDB", 0x68}, {"LDCH", 0x50}, {"LDF", 0x70}, {"LDL", 0x08}, {"LDS", 0x6C}, {"LDT", 0x74}, {"LDX", 0x04}, {"LPS", 0xD0}, {"MUL", 0x20}, {"MULF", 0x60}, {"MULR", 0x98}, {"NORM", 0xC8}, {"OR", 0x44}, {"RD", 0xD8}, {"RMO", 0xAC}, {"RSUB", 0x4C}, {"SHIFTL", 0xA4}, {"SHIFTR", 0xA8}, {"SIO", 0xF0}, {"SSK", 0xEC}, {"STA", 0x0C}, {"STB", 0x78}, {"STCH", 0x54}, {"STF", 0x80}, {"STI", 0xD4}, {"STL", 0x14}, {"STS", 0x7C}, {"STSW", 0xE8}, {"STT", 0x84}, {"STX", 0x10}, {"SUB", 0x1C}, {"SUBF", 0x5C}, {"SUBR", 0x94}, {"SVC", 0xB0}, {"TD", 0xE0}, {"TIO", 0xF8}, {"TIX", 0x2C}, {"TIXR", 0xB8}, {"WD", 0xDC}
};

const unordered_map<string, set<unsigned char>> SICXEAssembler::format_table = {
    // SIC/XE format
    // format 1 (1 byte)
    // format 2 (2 bytes)
    // format 3 (3 bytes)
    // format 4 (4 bytes)
    {"ADD", {3, 4}}, {"ADDF", {3, 4}}, {"ADDR", {2}}, {"AND", {3, 4}}, {"CLEAR", {2}}, {"COMP", {3, 4}}, {"COMPF", {3, 4}}, {"COMPR", {2}}, {"DIV", {3, 4}}, {"DIVF", {3, 4}}, {"DIVR", {2}}, {"FIX", {1}}, {"FLOAT", {1}}, {"HIO", {1}}, {"J", {3, 4}}, {"JEQ", {3, 4}}, {"JGT", {3, 4}}, {"JLT", {3, 4}}, {"JSUB", {3, 4}}, {"LDA", {3, 4}}, {"LDB", {3, 4}}, {"LDCH", {3, 4}}, {"LDF", {3, 4}}, {"LDL", {3, 4}}, {"LDS", {3, 4}}, {"LDT", {3, 4}}, {"LDX", {3, 4}}, {"LPS", {3, 4}}, {"MUL", {3, 4}}, {"MULF", {3, 4}}, {"MULR", {2}}, {"NORM", {1}}, {"OR", {3, 4}}, {"RD", {3, 4}}, {"RMO", {2}}, {"RSUB", {3, 4}}, {"SHIFTL", {2}}, {"SHIFTR", {2}}, {"SIO", {1}}, {"SSK", {3, 4}}, {"STA", {3, 4}}, {"STB", {3, 4}}, {"STCH", {3, 4}}, {"STF", {3, 4}}, {"STI", {3, 4}}, {"STL", {3, 4}}, {"STS", {3, 4}}, {"STSW", {3, 4}}, {"STT", {3, 4}}, {"STX", {3, 4}}, {"SUB", {3, 4}}, {"SUBF", {3, 4}}, {"SUBR", {2}}, {"SVC", {2}}, {"TD", {3, 4}}, {"TIO", {1}}, {"TIX", {3, 4}}, {"TIXR", {2}}, {"WD", {3, 4}}
};

const unordered_map<string, unsigned char> SICXEAssembler::register_table = {
    {"A", 0x00}, {"X", 0x01}, {"L", 0x02}, {"B", 0x03}, {"S", 0x04}, {"T", 0x05}, {"F", 0x06}, {"PC", 0x08}, {"SW", 0x09}
};

bool SICXEAssembler::isExtended(string opcode) {
    return opcode[0] == '+';
}

string SICXEAssembler::getOpcode(string extended_opcode) {
    return extended_opcode.substr(1);
}

SICXEAssembler::instruction SICXEAssembler::process_instruction(int &locctr, string &label, string &opcode, string &operand) {
    instruction _i;
    _i.label = label;
    _i.opcode = opcode;
    _i.operand = operand;
    _i.address = locctr;
    _i.length = 0;

    if(label != "") {
        if(this->symbol_table.find(label) != this->symbol_table.end()) {
            // duplicate symbol
            this->error_flag |= 4;
        } else {
            this->symbol_table[label] = locctr;
        }
    }

    set<unsigned char> formats;
    if(opcode == "WORD") {
        _i.length = 3;
    } else if(opcode == "RESW") {
        _i.length = 3 * _stoi(operand, 10);
    } else if(opcode == "RESB") {
        _i.length = _stoi(operand);
    } else if(opcode == "BYTE") {
        if(toupper(operand[0]) == 'C') {
            _i.length = operand.length() - 3;
        } else if(toupper(operand[0]) == 'X') {
            _i.length = (operand.length() - 3) / 2;
        } else {
            // invalid operand
            this->error_flag |= 8;
        }
    } else if(opcode_table.find(opcode) != opcode_table.end()) {
        formats = format_table.at(opcode);
        if(formats.find(1) != formats.end()) {
            _i.length = 1;
        } else if(formats.find(2) != formats.end())
            _i.length = 2;
        else if(formats.find(3) != formats.end())
            _i.length = 3;
        else 
            this->error_flag |= 16;
    } else if(isExtended(opcode) && opcode_table.find(getOpcode(opcode)) != opcode_table.end()) {
        _i.length = 4;
    } else if(opcode == "START") {
        this->start_address = locctr = _i.address = _stoi(operand, 16);
    } else if(opcode == "END") {
        this->program_length = locctr - this->start_address;
    } else if(opcode == "BASE" || opcode == "NOBASE") {
        // do nothing
    } else {
        // invalid opcode
        this->error_flag |= 16;
    }

    locctr += _i.length;

    return _i;
}

void SICXEAssembler::write_intermediate_line(int &line_number, instruction &processed_instruction) const {
    // every element must align to 10 characters
    string element, line = "";
    line += align_right(to_string(line_number * 5), 10, ' ') + "\t";
    line += align_right(((processed_instruction.opcode == "END" || processed_instruction.opcode == "BASE" || processed_instruction.opcode == "NOBASE") ? "" : itos(processed_instruction.address, 16)), 10, ' ') + "\t";
    line += align_right(processed_instruction.label, 10, ' ') + "\t";
    line += align_right(processed_instruction.opcode, 10, ' ') + "\t";
    line += align_right(processed_instruction.operand, 10, ' ') + "\n";
    this->intermediate->write(line.c_str(), line.length());
}

void SICXEAssembler::write_comment(int &line_number, string &comment) const {
    string line = "";
    line += align_right(to_string(line_number * 5), 10) + "\t";
    line += string(10, ' ') + "\t" + comment + "\n";
    this->intermediate->write(line.c_str(), line.length());
}

SICXEAssembler::SICXEAssembler(InputStream *input, OutputStream *output_object, fstream *intermediate, OutputStream *output_listing) {
    this->input = input;
    this->output_object = output_object;
    this->intermediate = intermediate;
    intermediate->seekp(0, ios_base::beg);
    this->output_listing = output_listing;
}

OutputStream* SICXEAssembler::fake_output_stream = new NoneOutputStream();

bool SICXEAssembler::pass1() {
    string line, opcode, operand, label;
    instruction processed_instruction;
    int locctr, line_number = 0;
    bool first_line = true;

    // restart intermediate file
    this->intermediate->seekp(0, ios::beg);
    this->program_length = 0;
    this->error_flag = 0;
    this->symbol_table = unordered_map<string, int>();
    while(true) {
        if(input->eof()) { // empty file
            this->error_flag |= 1;
            return false;
        }
        line = input->readline();

        if(!this->input_is_comment(line)) break;
        else {
            line_number++;
            this->write_comment(line_number, line);
        }
    }

    if(parse_input_line(line, label, opcode, operand)){
        if(opcode == "START"){
            first_line = false;
            processed_instruction = this->process_instruction(locctr, label, opcode, operand);
            if(this->error_flag) return false;
            line_number++;
            this->write_intermediate_line(line_number, processed_instruction);
        } else if(opcode == "END") { // empty program
            this->error_flag |= 1;
            return false;
        } else {
            locctr = this->start_address = 0;
        }
    } else { // invalid line
        this->error_flag |= 2;
        return false;
    }

    if(first_line) {
        processed_instruction = this->process_instruction(locctr, label, opcode, operand);
        if(this->error_flag) return false;
        line_number++;
        this->write_intermediate_line(line_number, processed_instruction);
    }

    while(!input->eof()) {
        line = input->readline();
        if(!this->input_is_comment(line)) {
            if(parse_input_line(line, label, opcode, operand)){
                if(opcode == "END"){
                    processed_instruction = this->process_instruction(locctr, label, opcode, operand);
                    if(this->error_flag) return false;
                    line_number++;
                    this->write_intermediate_line(line_number, processed_instruction);
                    return true;
                } else {
                    processed_instruction = this->process_instruction(locctr, label, opcode, operand);
                    if(this->error_flag) return false;
                    line_number++;
                    this->write_intermediate_line(line_number, processed_instruction);
                }
            } else { // invalid line
                this->error_flag |= 2;
                return false;
            }
        } else {
            line_number++;
            this->write_comment(line_number, line);
        }
    }

    // no END statement
    this->error_flag |= 32;
    return false;
}

bool SICXEAssembler::pass2() {
    int line_number, address;
    string line, opcode, operand, label, object_code, h_record, e_record, tmp_s;
    text_record t_record;
    instruction processed_instruction;
    bool first_line = true;

    // restart intermediate file
    this->intermediate->seekp(0, ios::beg);
    this->base = -1;
    this->m_records.clear();
    this->error_flag = 0;
    while(true) {
        if(intermediate->eof()) { // empty file
            this->error_flag |= 64 | 1;
            return false;
        }
        getline(*this->intermediate, line);

        if(!this->intermediate_is_comment(line)) break;
        else this->output_listing->write(line + '\n');
    }

    if(parse_intermediate_line(line, line_number, address, label, opcode, operand)){
        if(opcode == "START"){
            first_line = false;

            h_record = "H" + sep() + label + '\t' + sep() + align_right(operand, 6, '0') + sep() + align_right(itos(this->program_length, 16), 6, '0') + '\n';
            this->output_object->write(h_record);
            this->write_listing_line(line, object_code);
        } else if(opcode == "END") { // empty program
            this->error_flag |= 64 | 1;
            return false;
        } else {
            h_record = "H" + sep() + "      " + '\t' + sep() + "000000" + sep() + align_right(itos(this->program_length, 16), 6, '0') + '\n';
            this->output_object->write(h_record);
        }
    } else { // invalid line
        this->error_flag |= 64 | 2;
        return false;
    }

    t_record = initialize_text_record(address);

    if(first_line) {
        if(opcode == "BASE") {
            object_code = "";
            this->base = symbol_table.at(operand);
        } else if(opcode == "NOBASE") {
            object_code = "";
            this->base = -1;
        } else {
            object_code = this->toObjCode(address, opcode, operand);
            this->process_text_record(t_record, address, object_code);
        }

        this->write_listing_line(line, object_code);
        if(this->error_flag) return false;
    }

    while(!intermediate->eof()) {
        getline(*this->intermediate, line);
        if(!this->intermediate_is_comment(line)) {
            if(parse_intermediate_line(line, line_number, address, label, opcode, operand)){
                if(opcode == "END"){
                    object_code = "";
                    if(t_record.length > 0) {
                        this->write_text_record(t_record);
                    }
                    this->write_listing_line(line, object_code);

                    // write modification records
                    for(int i = 0; i < m_records.size(); i++) {
                        tmp_s = "M" + sep() + align_right(itos(m_records[i].address, 16), 6, '0') + sep() + align_right(itos(m_records[i].length, 16), 2, '0') + '\n';
                        this->output_object->write(tmp_s);
                    }

                    e_record = "E" + sep() + align_right(itos(this->start_address, 16), 6, '0') + '\n';
                    this->output_object->write(e_record);
                    return true;
                } else {
                    if(opcode == "BASE") {
                        object_code = "";
                        this->base = symbol_table.at(operand);
                    } else if(opcode == "NOBASE") {
                        object_code = "";
                        this->base = -1;
                    } else {
                        object_code = this->toObjCode(address, opcode, operand);
                        this->process_text_record(t_record, address, object_code);
                    }

                    this->write_listing_line(line, object_code);
                    if(this->error_flag) return false;
                }
            } else { // invalid line
                this->error_flag |= 64 | 2;
                return false;
            }
        } else {
            this->output_listing->write(line + '\n');
        }
    }

    // no END statement
    this->error_flag |= 64 | 32;
    return false;
}

string SICXEAssembler::toObjCode(int &locctr, string &opcode, string &operand) {
    #define flag_n 32
    #define flag_i 16
    #define flag_x 8
    #define flag_b 4
    #define flag_p 2
    #define flag_e 1
    string objCode, tmp_s;
    int opcode_i, flags = 0, disp = 0;
    set<unsigned char> format;
    vector<string> operands;

    if(isExtended(opcode) && opcode_table.find(getOpcode(opcode)) != opcode_table.end()) {
        opcode_i = opcode_table.at(getOpcode(opcode));
        flags |= flag_e;
        if(operand == "") {
            flags |= flag_n | flag_i;
            objCode = align_right(itos(opcode_i << 24 | flags << 20, 16), 8, '0');
        } else {
            if(isIndexed(operand)) {
                flags |= flag_n | flag_i | flag_x;
                disp = getAddress(locctr, operand.substr(0, operand.length() - 2));
            } else if(isImmediate(operand)) {
                flags |= flag_i;
                disp = getAddress(locctr, operand.substr(1));
            } else if(isIndirect(operand)) {
                flags |= flag_n;
                disp = getAddress(locctr, operand.substr(1));
            } else {
                flags |= flag_n | flag_i;
                disp = getAddress(locctr, operand);
            }

            objCode = align_right(itos(opcode_i << 24 | flags << 20 | disp, 16), 8, '0');
        }
    } else if(opcode_table.find(opcode) != opcode_table.end()) {
        format = format_table.at(opcode);
        if(format.find(1) != format.end()) {
            objCode = align_right(itos(opcode_table.at(opcode), 16), 2, '0');
        } else if(format.find(2) != format.end()) {
            operands = split(operand, ",");
            opcode_i = opcode_table.at(opcode);
            if(operands.size() == 2) {
                if(register_table.find(operands[0]) != register_table.end() && register_table.find(operands[1]) != register_table.end()) {
                    objCode = align_right(itos(opcode_i << 8 | register_table.at(operands[0]) << 4 | register_table.at(operands[1]), 16), 4, '0');
                } else {
                    this->error_flag |= 64 | 8;
                    objCode = "";
                }
            } else if(operands.size() == 1) {
                if(register_table.find(operands[0]) != register_table.end()) {
                    objCode = align_right(itos(opcode_i << 8 | register_table.at(operands[0]) << 4, 16), 4, '0');
                } else {
                    this->error_flag |= 64 | 8;
                    objCode = "";
                }
            } else {
                this->error_flag |= 64 | 8;
                objCode = "";
            }
        } else {
            opcode_i = opcode_table.at(opcode);
            if(operand == "") {
                flags |= flag_n | flag_i;
                objCode = align_right(itos(opcode_i << 16 | flags << 12, 16), 6, '0');
            } else {
                locctr += 3;
                if(isIndexed(operand)) {
                    flags |= flag_n | flag_i | flag_x;
                    disp = getDisplacement(locctr, flags, operand.substr(0, operand.length() - 2));
                } else if(isImmediate(operand)) {
                    flags |= flag_i;
                    disp = getDisplacement(locctr, flags, operand.substr(1));
                } else if(isIndirect(operand)) {
                    flags |= flag_n;
                    disp = getDisplacement(locctr, flags, operand.substr(1));
                } else {
                    flags |= flag_n | flag_i;
                    disp = getDisplacement(locctr, flags, operand);
                }

                locctr -= 3;
                objCode = align_right(itos(opcode_i << 16 | flags << 12 | disp, 16), 6, '0');
            }
        }
    } else if(opcode == "BYTE") {
        if(toupper(operand[0]) == 'C') {
            for(int i = 2; i < operand.length() - 1; i++) {
                objCode += align_right(itos(operand[i], 16), 2, '0');
            }
        } else if(toupper(operand[0]) == 'X') {
            objCode = operand.substr(2, operand.length() - 3);
        } else { // invalid operand
            this->error_flag |= 64 | 8;
            objCode = "";
        }
    } else if(opcode == "WORD") {
        disp = _stoi(operand, 10);
        if(disp < 0) disp += 1 << 24;
        objCode = align_right(itos(disp, 16), 6, '0');
    } else if(opcode == "RESB" || opcode == "RESW") {
        objCode = "";
    } else { // invalid opcode
        this->error_flag |= 64 | 16;
        objCode = "";
    }

    return objCode;
}

int SICXEAssembler::getAddress(int &locctr, string operand) {
    if(isNumber(operand)) {
        return _stoi(operand);
    } else if(symbol_table.find(operand) != symbol_table.end()) {
        modification_record m_record;
        m_record.address = locctr + 1;
        m_record.length = 5;
        this->m_records.push_back(m_record);
        return symbol_table.at(operand);
    } else {
        this->error_flag |= 64 | 4;
        return 0;
    }
}

int SICXEAssembler::getDisplacement(int &locctr, int &flags, string operand) {
    int disp = 0;
    if(isNumber(operand)) {
        disp = _stoi(operand);
    } else if(symbol_table.find(operand) != symbol_table.end()) {
        disp = symbol_table.at(operand);
        if(disp - locctr >= -2048 && disp - locctr <= 2047) { // Use PC relative
            flags |= flag_p;
            disp -= locctr;
            if(disp < 0) disp += 1 << 12;
        } else if(this->base != -1 && disp - this->base >= 0 && disp - this->base <= 4095) { // Use base relative
            flags |= flag_b;
            disp -= this->base;
        } else {
            this->error_flag |= 64 | 8;
            disp = 0;
        }
    } else {
        this->error_flag |= 64 | 8;
        disp = 0;
    }

    return disp;
}

bool SICXEAssembler::isImmediate(string operand) {
    return operand[0] == '#';
}

bool SICXEAssembler::isIndirect(string operand) {
    return operand[0] == '@';
}

bool SICXEAssembler::isIndexed(string operand) {
    return operand[operand.length() - 2] == ',' && operand[operand.length() - 1] == 'X';
}

void SICXEAssembler::process_text_record(text_record &t_record, int &address, string &obj_code) {
    if(t_record.start_address + t_record.length < address) {
        if(obj_code != "") {
            this->write_text_record(t_record);
            t_record = initialize_text_record(address);
        } else return;
    }

    string obj_code_tmp = obj_code;
    int tmp;
    while(t_record.length * 2 + obj_code_tmp.length() > 60) {
        tmp = 60 - t_record.length * 2;
        if(tmp > 6) {
            t_record.object_codes += obj_code_tmp.substr(0, tmp);
            obj_code_tmp = obj_code_tmp.substr(tmp, obj_code_tmp.length() - tmp);
            t_record.length += tmp / 2;
        }
        this->write_text_record(t_record);
        tmp = t_record.start_address + t_record.length;
        t_record = initialize_text_record(tmp);
    }
    t_record.object_codes += obj_code_tmp;
    t_record.length += obj_code_tmp.length() / 2;
}

void SICXEAssembler::write_text_record(text_record &t_record) const {
    this->output_object->write("T" + sep() + align_right(itos(t_record.start_address, 16), 6, '0')
    + sep() + align_right(itos(t_record.length, 16), 2, '0') + sep() + t_record.object_codes + '\n');
}

void SICXEAssembler::write_listing_line(string &intermediate_line, string &obj_code) const {
    this->output_listing->write(intermediate_line + '\t' + align_right(obj_code, 10, ' ') + '\n');
}

bool SICXEAssembler::assemble() {
    if (!pass1()) {
        return false;
    }

    if (!pass2()) {
        return false;
    }

    return true;
}

bool SICXEAssembler::parse_input_line(string line, string& label, string& opcode, string& operand) {
    // split 'line' into 'label', 'opcode', and 'operand'
    // return true if parsing is successful, false otherwise
    // if 'line' is empty, return false
    line = dealign_left(line, '\t');
    
    vector<string> tokens;
    string token = "";
    for(int i = 0; i < line.length(); i++) {
        if(!isSpace(line[i])) {
            token += line[i];
        } else {
            if(tokens.size() >= 2) {
                token += line[i];
            } else if(token != "") {
                tokens.push_back(token);
                token = "";
            }
        }
    }

    token = dealign_left(dealign_right(token, ' '), ' ');
    if(token != "") tokens.push_back(token);

    if(tokens.size() == 0) return false;
    if(tokens.size() == 1) {
        label = "";
        opcode = upper(tokens[0]);
        operand = "";
    } else if(tokens.size() == 2) {
        if(opcode_table.find(upper(tokens[0])) != opcode_table.end() || (isExtended(tokens[0]) && opcode_table.find(getOpcode(tokens[0])) != opcode_table.end()) || upper(tokens[0]) == "START" || upper(tokens[0]) == "END" || upper(tokens[0]) == "BASE" || upper(tokens[0]) == "NOBASE") {
            label = "";
            opcode = upper(tokens[0]);
            operand = tokens[1];
        } else {
            if(opcode_table.find(upper(tokens[1])) != opcode_table.end() || (isExtended(tokens[1]) && opcode_table.find(getOpcode(tokens[1])) != opcode_table.end()) || upper(tokens[1]) == "START" || upper(tokens[1]) == "END" || upper(tokens[1]) == "BASE" || upper(tokens[1]) == "NOBASE") {
                label = tokens[0];
                opcode = upper(tokens[1]);
                operand = "";
            } else return false;
        }
    } else {
        label = tokens[0];
        opcode = upper(tokens[1]);
        operand = tokens[2];
    }

    return true;
}

bool SICXEAssembler::input_is_comment(string line) {
    // return true if 'line' is a comment, false otherwise
    return line[0] == '.' || line == "";
}

bool SICXEAssembler::parse_intermediate_line(string line, int &line_number, int& address, string& label, string& opcode, string& operand) {
    vector<string> tokens = split(line, "\t");
    if(tokens.size() != 5) return false;
    line_number = _stoi(dealign_right(tokens[0], ' '), 10);
    address = _stoi(dealign_right(tokens[1], ' '), 16);
    label = dealign_right(tokens[2], ' ');
    opcode = dealign_right(tokens[3], ' ');
    operand = dealign_right(tokens[4], ' ');
    if(opcode == "") return false;
    return true;
}

bool SICXEAssembler::intermediate_is_comment(string line) {
    // return true if 'line' is a comment, false otherwise
    int i = 0;
    for(; i < line.length() && line[i] != '\t'; i++);
    if(i >= line.length()) return true;
    else {
        i++;
        for(; i < line.length() && line[i] != '\t'; i++);
        if(i >= line.length()) return true;
        else {
            return line[i+1] == '.';
        }
    }
}

SICXEAssembler::text_record SICXEAssembler::initialize_text_record(int address) {
    // return the initialized text record
    text_record t_record;
    t_record.start_address = address;
    t_record.length = 0;
    t_record.object_codes = "";
    return t_record;
}

void SICXEAssembler::setInputStream(InputStream *input) {
    this->input = input;
}

void SICXEAssembler::setOutputObjectStream(OutputStream *output_object) {
    this->output_object = output_object;
}

void SICXEAssembler::setIntermediateStream(fstream *intermediate) {
    this->intermediate = intermediate;
}

void SICXEAssembler::setOutputListingStream(OutputStream *output_listing) {
    this->output_listing = output_listing;
}

void SICXEAssembler::setSymbolTable(unordered_map<string, int> symbol_table) {
    this->symbol_table = symbol_table;
}

void SICXEAssembler::setProgramLength(int program_length) {
    this->program_length = program_length;
}

InputStream *SICXEAssembler::getInputStream() {
    return this->input;
}

OutputStream *SICXEAssembler::getOutputObjectStream() {
    return this->output_object;
}

fstream *SICXEAssembler::getIntermediateStream() {
    return this->intermediate;
}

OutputStream *SICXEAssembler::getOutputListingStream() {
    return this->output_listing;
}

unordered_map<string, int> SICXEAssembler::getSymbolTable() {
    return this->symbol_table;
}

int SICXEAssembler::getProgramLength() {
    return this->program_length;
}

int SICXEAssembler::getErrorFlag() {
    return this->error_flag;
}
