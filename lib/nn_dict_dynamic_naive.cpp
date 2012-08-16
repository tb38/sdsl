#ifndef INCLUDED_NN_DICT_DYNAMIC_NAIVE
#define INCLUDED_NN_DICT_DYNAMIC_NAIVE

#include <sdsl/int_vector.hpp>

namespace sdsl{

class nn_dict_dynamic_naive; // forward declaration

namespace util
{
void set_zero_bits(nn_dict_dynamic_naive& nn);
}

//! A class for a dynamic bit vector which also supports the prev and next operations
class nn_dict_dynamic_naive
{
    public:
        typedef int_vector<64>::size_type size_type;
        class reference; // forward declaration of inner class

        friend class reference;
        friend void util::set_zero_bits(nn_dict_dynamic_naive &nn);
    private:
        size_type m_size;
        int_vector<64> m_vector;

        void copy(const nn_dict_dynamic_naive &nn) {
            m_size = nn.m_size;
            m_vector = nn.m_vector;
        }

    public:
        size_type size() const {
            return m_size;
        }

        //! Constructor
        /*! \param n Number of supported bits
         */
        nn_dict_dynamic_naive(const uint64_t n = 0) {
            m_size = n;
            m_vector = int_vector<64>((n>>6)+1);
        }

        //! Copy constructor
        nn_dict_dynamic_naive(const nn_dict_dynamic_naive &nn) {
            copy(nn);
        }

        //! Assignment operator
        nn_dict_dynamic_naive& operator=(const nn_dict_dynamic_naive &nn) {
            if( this != &nn ) {
                copy(nn);
            }
            return *this;
        }

        void swap(nn_dict_dynamic_naive &nn) {
            if( this != &nn ) {
                std::swap(m_size, nn.m_size);
                m_vector.swap(nn.m_vector);
            }
        }

        //! Resize the dynamic bit vector in terms of elements.
        /*! \param size The size to resize the dynamic bit vector in terms of elements.
         *
         *  Required for the Sequence Concept of the STL.
         */
        void resize(const size_type size) {
            m_size = size;
            m_vector.resize((size>>6)+1);
        }

        //! Access the bit at index idx
        /*! \param idx Index
         *  \par Precondition
         *    \f$ 0 \leq  idx < size() \f$
         */
        bool operator[](const size_type& idx) const {
            uint64_t node = m_vector[idx>>6];
            return (node >> (idx&0x3F)) & 1;
        }

        inline reference operator[](const size_type& idx) {
            return reference(this, idx);
        }


        //! Get the leftmost index \f$i\geq idx\f$ where a bit is set.
        /*! \param idx Left border of the search interval. \f$ 0\leq idx < size()\f$
         *
         *  \return If there exists a leftmost index \f$i\geq idx\f$ where a bit is set,
         *          then \f$i\f$ is returned, otherwise size().
         */
        size_type next(const size_type idx) const {
            uint64_t pos = idx>>6;
            uint64_t node = m_vector[pos];
            node >>= (idx&0x3F);
            if(node) {
                return bit_magic::r1BP(node)+((pos<<6)|(idx&0x3F));
            } else {
                ++pos;
                while(pos < m_vector.size() ) {
                    if(m_vector[pos]) {        //m_vector[pos])+(pos<<6
                        return bit_magic::r1BP(m_vector[pos])|(pos<<6);
                    }
                    ++pos;
                }
                return size();
            }
        }

        //! Get the rightmost index \f$i \leq idx\f$ where a bit is set.
        /*! \param idx Right border of the search interval. \f$ 0 \leq idx < size()\f$
         *
         *  \return If there exists a rightmost index \f$i \leq idx\f$ where a bit is set,
         *          then \f$i\f$ is returned, otherwise size().
         */
        size_type prev(const size_type idx) const {
            uint64_t pos = idx>>6;
            uint64_t node = m_vector[pos];
            node <<= 63-(idx&0x3F);
            if(node) {
                return bit_magic::l1BP(node)+(pos<<6)-(63-(idx&0x3F));
            } else {
                --pos;
                while(pos < m_vector.size() ) {
                    if(m_vector[pos]) {//       (node)+(pos<<6);
                        return bit_magic::l1BP(node)|(pos<<6);
                    }
                    --pos;
                }
                return size();
            }
        }

        //! Load the data structure
        void load(std::istream& in) {
            util::read_member(m_size, in);
            m_vector.load(in);
        }

        //! Serialize the data structure
        size_type serialize(std::ostream& out, structure_tree_node* v=NULL, std::string name="")const {
            structure_tree_node* child = structure_tree::add_child(v, name, util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += util::write_member(m_size, out, child, "size");
            written_bytes += m_vector.serialize(out, child, "vector");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

#ifdef MEM_INFO
        //! Print some infos about the size of the data structure
        void mem_info() {
            // TODO
        }
#endif

    class reference {
        private:
            nn_dict_dynamic_naive * m_pbv; // pointer to the bit_vector_nearest_neigbour
            size_type m_idx;     // virtual node position
        public:
            //! Constructor
            reference(nn_dict_dynamic_naive *pbv,
                    nn_dict_dynamic_naive::size_type idx):m_pbv(pbv),m_idx(idx) {};

            //! Assignment operator for the proxy class
            reference& operator=(bool x) {
                if(x) {
                    m_pbv->m_vector[m_idx>>6] |= (1ULL<<(m_idx & 0x3F));
                } else {
                    m_pbv->m_vector[m_idx>>6] &= ~(1ULL<<(m_idx & 0x3F));
                }
                return *this;
            }

            reference& operator=(const reference& x) {
                return *this = bool(x);
            }

            //! Cast the reference to a bool
            operator bool() const {
                uint64_t node = m_pbv->m_vector[m_idx>>6];
                return (node>>(m_idx & 0x3F)) & 1;
            }

            bool operator==(const reference& x) const {
                return bool(*this) == bool(x);
            }

            bool operator<(const reference&x) const {
                return !bool(*this) and bool(x);
            }
    };

};

namespace util {
    void set_zero_bits(nn_dict_dynamic_naive& nn) {
        util::set_zero_bits(nn.m_vector);
    }
}

} // end of namespace

#endif // end file