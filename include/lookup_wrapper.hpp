#pragma once

//#include "query/algorithm/extern_query.hpp"
//#include "query/algorithm.hpp"
//#include "pisa/tools/app.hpp"
//#include "wand_data_raw.hpp"
//#include "wand_data_compressed.hpp"

//using namespace pisa;

namespace candidates
{
//    struct term_and_didlist
//    {
//        const Query single_query;
//        std::vector<uint64_t> vec_target_did;
//    };
//
//    class lookup_wrapper
//    {
//        using wand_raw_index = wand_data<wand_data_raw>;
//        using wand_uniform_index = wand_data<wand_data_compressed<>>;
//        using wand_uniform_index_quantized = wand_data<wand_data_compressed<PayloadType::Quantized>>;
//        using query_function = std::function<std::vector<typename topk_queue::entry_type>(Query)>;
//        using scorers = std::unique_ptr<index_scorer<wand_data<wand_data_raw>>, std::default_delete<index_scorer<wand_data<wand_data_raw>>>>;
//        using documents = Payload_Vector<std::string_view>;
//        using sources = shared_ptr<mio::mmap_source>;
//
//        public:
//            lookup_wrapper(App<arg::Index,
//                    arg::WandData<arg::WandMode::Required>,
//                    arg::Query<arg::QueryMode::Ranked>,
//                    arg::Algorithm,
//                    arg::Scorer,
//                    arg::Thresholds,
//                    arg::Threads> & app) : app(app)
//                    {
////                        this->index(MemorySource::mapped_file(app.index_filename()));
////                        this->wdata(MemorySource::mapped_file(app.wand_data_path()));
////                        this->scorer(scorer::from_params(app.scorer_params(), this->wdata));
////                        this->source(std::make_shared<mio::mmap_source>(documents_file.c_str()));
////                        this->docmap(Payload_Vector<>::from(*source));
//                    };
//            ~lookup_wrapper();
//            std::vector<short> lookup_byterm(string term)
//            {
//                return {};
//            };
//
//            template <typename IndexType, typename WandType>
//            short lookup_score(string did, string term)
//            {
//                return 0;
//            };
//
//        private:
//            App<arg::Index,
//            arg::WandData<arg::WandMode::Required>,
//            arg::Query<arg::QueryMode::Ranked>,
//            arg::Algorithm,
//            arg::Scorer,
//            arg::Thresholds,
//            arg::Threads> & app;
//    };

}