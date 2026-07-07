#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <sstream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Penggunaan: ./LogParser <input_mentah.log> <output.csv>\n";
        return 1;
    }

    string infile = argv[1];
    string outfile = argv[2];

    ifstream in(infile);
    if (!in.is_open()) return 1;

    vector<string> lines;
    string line;
    while (getline(in, line)) {
        lines.push_back(line);
    }
    in.close();

    if (lines.empty()) return 0;

    // 1. TAHAP PEMINDAIAN AWAL (Mencari jumlah Core CPU maksimal & Sensor Daya)
    int max_cores = 0;
    vector<string> power_sensors;

    regex cpu_rgx("CPU \\[([^\\]]+)\\]");
    regex pwr_rgx("\\b([A-Z0-9_]+)\\s+(\\d+)/\\d+");

    for (const auto& l : lines) {
        smatch m;
        // Hitung core CPU berdasarkan jumlah koma
        if (regex_search(l, m, cpu_rgx)) {
            string cpu_str = m[1].str();
            int cores = 1;
            for (char c : cpu_str) {
                if (c == ',') cores++;
            }
            if (cores > max_cores) max_cores = cores;
        }

        // Cari sensor Power
        auto pwr_begin = sregex_iterator(l.begin(), l.end(), pwr_rgx);
        auto pwr_end = sregex_iterator();
        for (sregex_iterator i = pwr_begin; i != pwr_end; ++i) {
            string p_name = (*i)[1].str();
            if (p_name != "RAM" && p_name != "SWAP") {
                bool exists = false;
                for (const auto& ps : power_sensors) {
                    if (ps == p_name) exists = true;
                }
                if (!exists) power_sensors.push_back(p_name);
            }
        }
    }

    // 2. BUAT HEADER CSV
    ofstream out(outfile);
    out << "Timestamp,RAM_MB,SWAP_MB,EMC_Persen,GPU_Persen";
    for (int i = 1; i <= max_cores; ++i) {
        out << ",CPU_Core_" << i << "_Persen,CPU_Core_" << i << "_Freq_MHz";
    }
    for (const auto& p : power_sensors) {
        out << "," << p << "_mW";
    }
    out << "\n";

    // 3. TAHAP EKSTRAKSI & PENULISAN BARIS DATA
    regex ts_rgx("^(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2})");
    regex ram_rgx("RAM (\\d+)/");
    regex swap_rgx("SWAP (\\d+)/");
    regex emc_rgx("EMC_FREQ (\\d+)%");
    regex gpu_rgx("GR3D_FREQ (\\d+)%");

    for (const auto& l : lines) {
        smatch m;
        string ts = regex_search(l, m, ts_rgx) ? m[1].str() : "";
        string ram = regex_search(l, m, ram_rgx) ? m[1].str() : "0";
        string swap = regex_search(l, m, swap_rgx) ? m[1].str() : "0";
        string emc = regex_search(l, m, emc_rgx) ? m[1].str() : "0";
        string gpu = regex_search(l, m, gpu_rgx) ? m[1].str() : "0";

        if (ts.empty()) continue; // Lewati baris yang rusak

        out << ts << "," << ram << "," << swap << "," << emc << "," << gpu;

        // Ekstrak CPU (Pisahkan Persentase dan Frekuensi)
        vector<pair<string, string>> core_data;
        if (regex_search(l, m, cpu_rgx)) {
            string cpu_str = m[1].str();
            stringstream ss(cpu_str);
            string token;
            while(getline(ss, token, ',')) {
                size_t at_pos = token.find('@');
                if(at_pos != string::npos) {
                    string perc = token.substr(0, at_pos);
                    if (!perc.empty() && perc.back() == '%') perc.pop_back();
                    string freq = token.substr(at_pos + 1);
                    core_data.push_back({perc, freq});
                } else {
                    core_data.push_back({"0", "0"});
                }
            }
        }
        
        // Tulis data CPU, isi 0 jika ada core yang sedang mode sleep/offline
        for (int i = 0; i < max_cores; ++i) {
            if (i < core_data.size()) {
                out << "," << core_data[i].first << "," << core_data[i].second;
            } else {
                out << ",0,0";
            }
        }

        // Ekstrak Data Power
        map<string, string> pwr_vals;
        auto pwr_begin = sregex_iterator(l.begin(), l.end(), pwr_rgx);
        auto pwr_end = sregex_iterator();
        for (sregex_iterator i = pwr_begin; i != pwr_end; ++i) {
            pwr_vals[(*i)[1].str()] = (*i)[2].str();
        }

        for (const auto& p : power_sensors) {
            if (pwr_vals.count(p)) out << "," << pwr_vals[p];
            else out << ",0";
        }
        out << "\n";
    }

    out.close();
    return 0;
}
