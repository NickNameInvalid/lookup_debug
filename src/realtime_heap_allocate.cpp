#include "../include/realtime_heap_allocate.h"
#include <algorithm>

namespace candidates {

    std::vector<std::string> realtime_heap_allocate::split(const std::string& str, const std::string& delim) {
//        std::vector<std::string> res;
//        if("" == str) return res;
//        char * strs = new char[str.length() + 1] ;
//        strcpy(strs, str.c_str());
//
//        char * d = new char[delim.length() + 1];
//        strcpy(d, delim.c_str());
//
//        char *p = strtok(strs, d);
//        while(p) {
//            std::string s = p;
//            res.push_back(s);
//            p = strtok(NULL, d);
//        }
//
//        return res;
        std::vector<std::string> res;
        if("" == str) return  res;

        std::string strs = str + delim;
        size_t pos;
        size_t size = strs.size();

        for (int i = 0; i < size; ++i) {
            pos = strs.find(delim, i);
            if( pos < size) {
                std::string s = strs.substr(i, pos - i);
                res.push_back(s);
                i = pos + delim.size() - 1;
            }

        }
        return res;
    }


    lexicon realtime_heap_allocate::load_lexicon(const std::string& gram_path, const std::string lexicon_path) {
        lexicon lex{};
        // std::fstream in_gram(gram_path, std::ios::in | std::ios::binary);
        //std::fstream in_lex(lexicon_path, std::ios::in | std::ios::binary);
        std::ifstream in_gram(gram_path);
        std::ifstream in_lex(lexicon_path);
        std::string gramline, lexline;
        std::vector<std::string> lex_pair;
        while(std::getline(in_gram, gramline))
        {
            std::getline(in_lex, lexline);
//            std::cout << lexline << '\n';
            boost::split(lex_pair, lexline, boost::is_any_of(" "), boost::token_compress_on);
//            for (auto v: lex_pair) {
//                std::cout << v << '\n';
//            }
            lex[gramline] = lexicon_offset(std::stoll(lex_pair[0], nullptr, 0), std::stoll(lex_pair[1], nullptr, 0));
        }
        in_gram.close();
        in_lex.close();
        return lex;
    }

    void realtime_heap_allocate::initialize() {
        clock_t start, end;
        start = clock();
        std::string single_gram_path = std::get<0>(this->structPath.sinind);
        std::string single_lexicon_path = std::get<1>(this->structPath.sinind);
        this->structLexicon.sinsublex = this->load_lexicon(single_gram_path, single_lexicon_path);
        end = clock();
        double running_time = double(end - start) / double(CLOCKS_PER_SEC);
        std::cout << "Time to Load Single Lexicon: " << running_time << "s\n";
        std::cout << this->structLexicon.sinsublex.size() << '\n';

        start = clock();
        std::string duplet_gram_path = std::get<0>(this->structPath.dupind);
        std::string duplet_lexicon_path = std::get<1>(this->structPath.dupind);
        this->structLexicon.dupsublex = this->load_lexicon(duplet_gram_path, duplet_lexicon_path);
        end = clock();
        running_time = double(end - start) / double(CLOCKS_PER_SEC);
        std::cout << "Time to Load Duplet Lexicon: " << running_time << "s\n";
        std::cout << this->structLexicon.dupsublex.size() << '\n';

        start = clock();
        std::string triplet_gram_path = std::get<0>(this->structPath.triind);
        std::string triplet_lexicon_path = std::get<1>(this->structPath.triind);
        this->structLexicon.trisublex = this->load_lexicon(triplet_gram_path, triplet_lexicon_path);
        end = clock();
        running_time = double(end - start) / double(CLOCKS_PER_SEC);
        std::cout << "Time to Load Triplet Lexicon: " << running_time << "s\n";
        std::cout << this->structLexicon.trisublex.size() << '\n';

        start = clock();
        std::string quadruplet_gram_path = std::get<0>(this->structPath.quadind);
        std::string quadruplet_lexicon_path = std::get<1>(this->structPath.quadind);
        this->structLexicon.quadsublex = this->load_lexicon(quadruplet_gram_path, quadruplet_lexicon_path);
        end = clock();
        running_time = double(end - start) / double(CLOCKS_PER_SEC);
        std::cout << "Time to Load Quadruplet Lexicon: " << running_time << "s\n";
        std::cout << this->structLexicon.quadsublex.size() << '\n';
    }

    inline lexicon_offset realtime_heap_allocate::get_offset(const std::string& substructure, lexicon & substructure_lex) {
        lexicon_offset offset{0, 0};
        if(substructure_lex.find(substructure) != substructure_lex.end())
        {
            offset = substructure_lex[substructure];
        }
        return offset;
    }

    std::unordered_map<int, std::vector<float>>
    realtime_heap_allocate::compute_hit_ratio(const std::string query, std::vector<int> & budget, const std::vector<int> & topk,
                                              std::bitset<4> & types, const std::string & metric) {
        std::cout << query << '\n';
        int max_budget = *std::max_element(std::begin(budget), std::end(budget));
        std::vector<std::string> query_terms = split(query, "_");

//        boost::split(query_terms, query, boost::is_any_of("_"), boost::token_compress_on);
        std::string queries = query_terms.back();
        query_terms = split(query, " ");
//        boost::split(query_terms, queries, boost::is_any_of(" "), boost::token_compress_on);
        auto [lst_sin, lst_dup, lst_tri, lst_quad] = this->get_comb(query_terms, types);
        std::priority_queue<postings> hp_posting = {};
        // std::cout << "A\n";

        // a global variable used for slice
        int num_to_slice;
        for(auto & single : lst_sin)
        {
            auto [start_pos, end_pos] = get_offset(single, this->structLexicon.sinsublex);
            if(start_pos && end_pos)
            {
                int64_t cur_pos = start_pos + 4;
                this->structInput.sinsub.seekg(cur_pos);
                int did;
                unsigned char score;
                this->structInput.sinsub.read(reinterpret_cast<char*>(&did), 4);
                this->structInput.sinsub.read(reinterpret_cast<char*>(&score), 1);
                hp_posting.push(postings(std::to_string(did), (short) score, 1, cur_pos + 5, end_pos));
            }
        }
        // std::cout << "B\n";
        for(auto & dup : lst_dup)
        {
            auto [start_pos, end_pos] = get_offset(dup, this->structLexicon.dupsublex);
            if(start_pos && end_pos)
            {
                int64_t cur_pos = start_pos + 4;
                this->structInput.dupsub.seekg(cur_pos);
                int did;
                std::pair<unsigned char, unsigned char> scores;
                this->structInput.dupsub.read(reinterpret_cast<char*>(&did), 4);
                this->structInput.dupsub.read(reinterpret_cast<char*>(&scores.first), 1);
                this->structInput.dupsub.read(reinterpret_cast<char*>(&scores.second), 1);
                short score = (short) scores.first + (short) scores.second;
//                std::cout << (short) scores.first << std::endl;
                hp_posting.push(postings(std::to_string(did), score, 2, cur_pos + 6, end_pos));
            }
        }
        // std::cout << "C\n";
        for(auto & tri : lst_tri)
        {
            auto [start_pos, end_pos] = get_offset(tri, this->structLexicon.trisublex);
            if(start_pos && end_pos)
            {
                int64_t cur_pos = start_pos + 4;
                this->structInput.trisub.seekg(cur_pos);
                int did;
                std::tuple<unsigned char, unsigned char, unsigned char> scores;
                this->structInput.trisub.read(reinterpret_cast<char*>(&did), 4);
                this->structInput.trisub.read(reinterpret_cast<char*>(&std::get<0>(scores)), 1);
                this->structInput.trisub.read(reinterpret_cast<char*>(&std::get<1>(scores)), 1);
                this->structInput.trisub.read(reinterpret_cast<char*>(&std::get<2>(scores)), 1);
//                std::cout << (short) std::get<0>(scores) << std::endl;
                short score = (short) std::get<0>(scores) + (short) std::get<1>(scores) + (short) std::get<2>(scores);
                hp_posting.push(postings(std::to_string(did), score, 3, cur_pos + 7, end_pos));
            }
        }
        // std::cout << "D\n";
        for(auto & quad : lst_quad)
        {
            auto [start_pos, end_pos] = get_offset(quad, this->structLexicon.quadsublex);
            if(start_pos && end_pos)
            {
                int64_t cur_pos = start_pos + 4;
                this->structInput.quadsub.seekg(cur_pos);
                int did;
                std::tuple<unsigned char, unsigned char, unsigned char, unsigned char> scores;
                this->structInput.quadsub.read(reinterpret_cast<char*>(&did), 4);
                this->structInput.quadsub.read(reinterpret_cast<char*>(&std::get<0>(scores)), 1);
                this->structInput.quadsub.read(reinterpret_cast<char*>(&std::get<1>(scores)), 1);
                this->structInput.quadsub.read(reinterpret_cast<char*>(&std::get<2>(scores)), 1);
                this->structInput.quadsub.read(reinterpret_cast<char*>(&std::get<3>(scores)), 1);
//                std::cout << (short) std::get<3>(scores) << std::endl;
                short score = (short) std::get<0>(scores) + (short) std::get<1>(scores) + (short) std::get<2>(scores) + (short) std::get<3>(scores);
                hp_posting.push(postings(std::to_string(did), score, 4, cur_pos + 8, end_pos));
            }
        }

        // std::cout << "E\n";
        std::vector<postings> pred_did{};
        while (!hp_posting.empty() && max_budget > 0)
        {
            postings cur_obj = hp_posting.top();
            pred_did.emplace_back(cur_obj);
            hp_posting.pop();
            max_budget--;

            if(cur_obj.ptr < cur_obj.ends)
            {
                int64_t cur_pos = cur_obj.ptr;
                int64_t end_pos = cur_obj.ends;
                if(cur_obj.type == 1)
                {
                    this->structInput.sinsub.seekg(cur_pos);
                    int did;
                    short score;
                    this->structInput.sinsub.read(reinterpret_cast<char*>(&did), 4);
                    this->structInput.sinsub.read(reinterpret_cast<char*>(&score), 1);
                    hp_posting.push(postings(std::to_string(did), (short) score, 1, cur_pos + 5, end_pos));
                }
                else if(cur_obj.type == 2)
                {
                    this->structInput.dupsub.seekg(cur_pos);
                    int did;
                    std::pair<short, short> scores;
                    this->structInput.dupsub.read(reinterpret_cast<char*>(&did), 4);
                    this->structInput.dupsub.read(reinterpret_cast<char*>(&scores.first), 1);
                    this->structInput.dupsub.read(reinterpret_cast<char*>(&scores.second), 1);
                    short score = (short) scores.first + (short) scores.second;
                    hp_posting.push(postings(std::to_string(did), score, 2, cur_pos + 6, end_pos));
                }
                else if(cur_obj.type == 3)
                {
                    this->structInput.trisub.seekg(cur_pos);
                    int did;
                    std::tuple<short, short, short> scores;
                    this->structInput.trisub.read(reinterpret_cast<char*>(&did), 4);
                    this->structInput.trisub.read(reinterpret_cast<char*>(&std::get<0>(scores)), 1);
                    this->structInput.trisub.read(reinterpret_cast<char*>(&std::get<1>(scores)), 1);
                    this->structInput.trisub.read(reinterpret_cast<char*>(&std::get<2>(scores)), 1);
                    short score = (short) std::get<0>(scores) + (short) std::get<1>(scores) + (short) std::get<2>(scores);
                    hp_posting.push(postings(std::to_string(did), score, 3, cur_pos + 7, end_pos));
                }
                else if(cur_obj.type == 4)
                {
                    this->structInput.quadsub.seekg(cur_pos);
                    int did;
                    std::tuple<short, short, short, short> scores;
                    this->structInput.quadsub.read(reinterpret_cast<char*>(&did), 4);
                    this->structInput.quadsub.read(reinterpret_cast<char*>(&std::get<0>(scores)), 1);
                    this->structInput.quadsub.read(reinterpret_cast<char*>(&std::get<1>(scores)), 1);
                    this->structInput.quadsub.read(reinterpret_cast<char*>(&std::get<2>(scores)), 1);
                    this->structInput.quadsub.read(reinterpret_cast<char*>(&std::get<3>(scores)), 1);
                    short score = (short) std::get<0>(scores) + (short) std::get<1>(scores) + (short) std::get<2>(scores) + (short) std::get<3>(scores);
                    hp_posting.push(postings(std::to_string(did), score, 4, cur_pos + 8, end_pos));
                }
            }
        }
        // std::cout << "F\n";
        int max_topk = *std::max_element(std::begin(topk), std::end(topk));
        std::vector<std::string> real_did{};
        std::ifstream output(query);
        int cnt_top = 0;
        std::string line;
        std::vector<std::string> line_splt;
        while (getline(output, line) && cnt_top < max_topk)
        {
            boost::split(line_splt, line, boost::is_any_of(" "), boost::token_compress_on);
            real_did.emplace_back(line_splt[1]);
            ++cnt_top;
        }
        output.close();
        std::unordered_map<int, std::vector<float>> query_acc = {};
//         std::cout << "G\n";

//        std::unordered_map<int, std::unordered_map<std::string, bool>> topk_dict;
//        for (auto i: topk) {
//            topk_dict[i] = {};
//        }
//
//        if(metric != "Recall") {
//            for (auto i: topk) {
//                std::vector<std::string> slice_real_did(real_did.begin(), real_did.begin() + i);
//                for (auto j: slice_real_did) {
//                    topk_dict[i][j] = true;
//                }
//            }
//        }
//
//        // std::cout << "H\n";
//
//
//        for(auto i : topk)
//        {
//            // std::cout << "Debug1\n";
//            // std::cout << real_did.size() << '\n';
//            // std::cout << pred_did.size() << '\n';
//            num_to_slice = std::min(i, int(real_did.size()));
//            std::vector<std::string> real_tmp(real_did.begin(), real_did.begin() + num_to_slice);
//            // std::cout << "current topk: " << i << '\n';
//            // std::cout << "Debug2\n";
//            // std::cout << budget.size() << '\n';
//            for(auto j : budget)
//            {
//                // std::cout << "current budget: " << j << '\n';
//                // std::vector<postings> lst_pred(pred_did.begin(), pred_did.begin() + j);
//                // std::cout << "Debug3\n";
//                std::vector<std::string> did_pred_sliced = {};
//                // std::cout << "Debug4\n";
//                num_to_slice = std::min(j, int(pred_did.size()));
//                for (int ite = 0; ite < num_to_slice; ++ite) {
//                    did_pred_sliced.emplace_back(pred_did[ite].did);
//                }
//                // std::cout << "Debug5\n";
//                query_acc[i].emplace_back(this->metric_score(metric, topk_dict[i], real_tmp, did_pred_sliced));
//                // std::cout << "Debug6\n";
//            }
//        }
//        std::cout << "I\n";
        return query_acc;
    }

    void realtime_heap_allocate::get_accuracy(std::string set_path, std::vector<int> & budget, std::vector<int> & topk, std::bitset<4> types, const std::string & metric) {
        std::vector<std::string> files{};
        for (const auto & entry : std::filesystem::directory_iterator(set_path))
            files.emplace_back(entry.path().string());
        std::unordered_map<int, std::vector<float>> avgacc;
        std::unordered_map<int, std::vector<std::vector<float>>> aux;

        for(auto & file : files)
        {
            std::unordered_map<int, std::vector<float>> dictacc = this->compute_hit_ratio(file, budget, topk, types, metric);
            this->query_name.emplace_back(file);
            for(auto & vk : dictacc)
            {
                for(int k = 0; k < budget.size(); ++k)
                {
                    if(vk.second.at(k) != 0)
                    {
                        aux[vk.first][k].emplace_back(vk.second[k]);
                    }
                }
            }
        }
        for(auto & vk : aux)
        {
            for(int k = 0; k < vk.second.size(); ++k)
            {
                avgacc[vk.first].emplace_back(std::accumulate(vk.second[k].begin(), vk.second[k].end(), 0) * 1.0 / vk.second[k].size());
            }
        }
        for(auto & vk : avgacc)
        {
            std::cout << "When k = " << vk.first << std::endl;
            for (auto i: vk.second)
                std::cout << i << " ";
        }
    }

    realtime_heap_allocate::realtime_heap_allocate(index_path single, index_path duplet, index_path triplet,
                                                   index_path quadruplet) {
        this->structPath.sinind = single;
        this->structPath.dupind = duplet;
        this->structPath.triind = triplet;
        this->structPath.quadind = quadruplet;

        this->structInput.sinsub.open(std::get<2>(single), std::ios::in | std::ios::binary);
        this->structInput.dupsub.open(std::get<2>(duplet), std::ios::in | std::ios::binary);
        this->structInput.trisub.open(std::get<2>(triplet), std::ios::in | std::ios::binary);
        this->structInput.quadsub.open(std::get<2>(quadruplet), std::ios::in | std::ios::binary);
    }


    comb_grams realtime_heap_allocate::get_comb(std::vector<std::string> &terms, std::bitset<4> & comb_type) {
        comb_grams comb = {};
        if(comb_type[0])
        {
            for (auto&& i : iter::combinations(terms, 1)) {
                std::string grams = boost::join(std::vector<std::string>(i.begin(), i.end()), " ");
                std::get<0>(comb).emplace_back(grams);
            }
        }
        if(comb_type[1])
        {
            for (auto&& i : iter::combinations(terms, 2)) {
                std::string grams = boost::join(std::vector<std::string>(i.begin(), i.end()), " ");
                std::get<1>(comb).emplace_back(grams);
            }
        }
        if(comb_type[2])
        {
            for (auto&& i : iter::combinations(terms, 3)) {
                std::string grams = boost::join(std::vector<std::string>(i.begin(), i.end()), " ");
                std::get<2>(comb).emplace_back(grams);
            }
        }
        if(comb_type[3])
        {
            for (auto&& i : iter::combinations(terms, 4)) {
                std::string grams = boost::join(std::vector<std::string>(i.begin(), i.end()), " ");
                std::get<3>(comb).emplace_back(grams);
            }
        }
        return comb;
    }

    float realtime_heap_allocate::metric_score(std::string metric, const std::unordered_map<std::string, bool> & topk_dict,
                                               std::vector<std::string> & topk_lst, std::vector<std::string> & topk_pred) {
        // std::cout << "calling metric_score\n";
        if(topk_lst.empty())
        {
            return 0;
        }
        // std::cout << "It is not zero\n";

        if(metric == "Recall")
        {
            std::vector<std::string> intersections;
            std::sort(topk_lst.begin(), topk_lst.end());
            std::sort(topk_pred.begin(), topk_pred.end());
//            for (auto real: topk_lst)
//                std::cout << real;
//            std::cout << '\n';
//            for (auto pred: topk_pred)
//                std::cout << pred;
//            std::cout << '\n';
            std::set_intersection(topk_lst.begin(),topk_lst.end(),topk_pred.begin(),topk_pred.end(), std::insert_iterator<std::vector<std::string>>(intersections,intersections.begin()));
            // std::cout << intersections.size() << '\n';
            return intersections.size() * 1.0 / topk_lst.size();
        }
        else if(metric == "MAP")
        {
            auto [lst_index, denominator, numerator] = std::tuple<int, int, int>{0, 0, 0};
            std::vector<float> precisions = {};
            std::unordered_map<std::string, bool> topk_included = {};
            while(lst_index < topk_pred.size())
            {
                ++denominator;
                if((topk_dict.find(topk_pred[lst_index]) != topk_dict.end()) &&
                    (topk_included.find(topk_pred[lst_index]) == topk_included.end()))
                {
                    topk_included[topk_pred[lst_index]] = true;
                    ++numerator;
                    precisions.emplace_back(numerator * 1.0 / denominator);
                }
            }
            return precisions.empty() ? 0 : std::accumulate(precisions.begin(), precisions.end(), 0) * 1.0 / precisions.size();
        }
        else if(metric == "MRR")
        {
            for(int i = 0; i < topk_pred.size(); ++i)
            {
                if(topk_dict.find(topk_pred[i]) != topk_dict.end())
                    return 1.0 / (i + 1);
            }
        }

        return 0;
    }

    realtime_heap_allocate::~realtime_heap_allocate()
    {
        this->structInput.sinsub.close();
        this->structInput.dupsub.close();
        this->structInput.trisub.close();
        this->structInput.quadsub.close();
    }

//    std::vector<postings>
//    realtime_heap_allocate::combine_postings(std::vector<postings> & postingslist) {
//        std::set<postings> set_pst(postingslist.begin(), postingslist.end());
//        std::vector<postings> comb_postings(set_pst.begin(), set_pst.end());
//        std::sort(comb_postings.begin(), comb_postings.end());
//        return {};
//    }

//    std::vector<short> realtime_heap_allocate::combine_twolists(std::vector<short> & list1, std::vector<short> & list2) {
//        std::vector<short> combined_lst(0, list1.size());
//        for(int i = 0; i < list1.size(); ++i)
//        {
//            combined_lst[i] = (list1[i] != 0 ? list1[i] : list2[i]);
//        }
//        return combined_lst;
//    }

//    postings realtime_heap_allocate::combine_substructures(std::vector<postings> & substructures) {
//        std::string did = substructures[0].did;
//        std::vector<std::string> terms(substructures[0].terms);
//        std::vector<short> termwise(substructures[0].termwise);
//        for(int i = 1; i < substructures.size(); ++i)
//        {
//            termwise = combine_twolists(termwise, substructures[i].termwise);
//            if (*std::min_element(std::begin(termwise), std::end(termwise)))
//                break;
//        }
//        return postings(did, terms, termwise);
//    }

//    std::tuple<std::vector<std::string>, int, int, std::vector<short>>
//    realtime_heap_allocate::lookup_allocation(std::vector<postings> &posting_from, int budget, int) {
//        return std::tuple<std::vector<std::string>, int, int, std::vector<short>>();
//    }

}