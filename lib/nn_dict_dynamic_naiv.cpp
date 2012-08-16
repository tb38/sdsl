#ifndef INCLUDED_NN_DICT_DYNAMIC_NAIV
#define INCLUDED_NN_DICT_DYNAMIC_NAIV

#include <sdsl/int_vector.hpp>
#include <sdsl/util.hpp>

namespace sdsl{

class nn_dict_dynamic_naiv; // forward declaration

namespace util
{
void set_zero_bits(nn_dict_dynamic_naiv& nn);
}

//! A class for a dynamic bit vector which also supports the prev and next operations
class nn_dict_dynamic_naiv
{
    public:
        typedef int_vector<64>::size_type size_type;
        class reference; // forward declaration of inner class

        friend class reference;
        friend void util::set_zero_bits(nn_dict_dynamic_naiv &nn);
    private:
        size_type m_size;
        int_vector<64> m_tree;             // Tree

        void copy(const nn_dict_dynamic_naiv &nn) {
            m_size = nn.m_size;
            m_tree = nn.m_tree;
        }

    public:
        size_type size() const {
            return m_size;
        }

        //! Constructor
        /*! \param n Number of supported bits
         */
        nn_dict_dynamic_naiv(const uint64_t n = 0) {
            m_size = n;
            m_tree = int_vector<64>((n>>6)+1);
        }

        //! Copy constructor
        nn_dict_dynamic_naiv(const nn_dict_dynamic_naiv &nn) {
            copy(nn);
        }

        //! Assignment operator
        nn_dict_dynamic_naiv& operator=(const nn_dict_dynamic_naiv &nn) {
            if( this != &nn ) {
                copy(nn);
            }
            return *this;
        }

        void swap(nn_dict_dynamic_naiv &nn) {
            if( this != &nn ) {
                std::swap(m_size, nn.m_size);
                m_tree.swap(nn.m_tree);
            }
        }

        //! Resize the dynamic bit vector in terms of elements.
        /*! \param size The size to resize the dynamic bit vector in terms of elements.
         *
         *  Required for the Sequence Concept of the STL.
         */
        void resize(const size_type size) {
            m_size = size;
            m_tree.resize((size>>6)+1);
        }

        //! Access the bit at index idx
        /*! \param idx Index
         *  \par Precondition
         *    \f$ 0 \leq  idx < size() \f$
         */
        bool operator[](const size_type& idx) const {
            uint64_t node = m_tree[idx>>6];
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
            uint64_t node = m_tree[pos];
            node >>= (idx&0x3F);
            if(node) {
                return bit_magic::r1BP(node)+((pos<<6)|(idx&0x3F));
            } else {
                ++pos;
                while(pos < m_tree.size() ) {
                    if(m_tree[pos]) {        //m_tree[pos])+(pos<<6
                        return bit_magic::r1BP(m_tree[pos])|(pos<<6);
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
            uint64_t node = m_tree[pos];
            node <<= 63-(idx&0x3F);
            if(node) {
                return bit_magic::l1BP(node)+(pos<<6)-(63-(idx&0x3F));
            } else {
                --pos;
                while(pos < m_tree.size() ) {
                    if(m_tree[pos]) {//       (node)+(pos<<6);
                        return bit_magic::l1BP(node)|(pos<<6);
                    }
                    --pos;
                }
                return size();
            }
        }


        //! Load the data structure
        void load(std::istream &in) {
            in.read((char*) &m_size, sizeof(m_size));
            m_tree.load(in);
        }

        //! Serialize the data structure
        size_type serialize(std::ostream &out) const {
            size_type written_bytes = 0;
            out.write((char*)&m_size, sizeof(m_size));
            written_bytes += sizeof(m_size);
            written_bytes += m_tree.serialize(out);
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
            nn_dict_dynamic_naiv * m_pbv; // pointer to the bit_vector_nearest_neigbour
            size_type m_idx;     // virtual node position
        public:
            //! Constructor
            reference(nn_dict_dynamic_naiv *pbv,
                    nn_dict_dynamic_naiv::size_type idx):m_pbv(pbv),m_idx(idx) {};

            //! Assignment operator for the proxy class
            reference& operator=(bool x) {
                if(x) {
                    m_pbv->m_tree[m_idx>>6] |= (1ULL<<(m_idx & 0x3F));
                } else {
                    m_pbv->m_tree[m_idx>>6] &= ~(1ULL<<(m_idx & 0x3F));
                }
                return *this;
            }

            reference& operator=(const reference& x) {
                return *this = bool(x);
            }

            //! Cast the reference to a bool
            operator bool() const {
                uint64_t node = m_pbv->m_tree[m_idx>>6];
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
    void set_zero_bits(nn_dict_dynamic_naiv& nn) {
        util::set_zero_bits(nn.m_tree);
    }
}

} // end of namespace

#endif // end file