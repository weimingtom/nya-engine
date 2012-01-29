#ifndef pool_h
#define pool_h

#include <vector>

namespace memory
{

template<typename t_data,int block_count> class pool
{
public:
	t_data *allocate()
	{
		long free_block;
		long free_offset;

		if(m_free_count<0)
		{
			block *b=new block();
			if (!b)
				return 0;

			const long prev_blocks_count = (long)m_blocks.size();
			const long first_idx = block_count * prev_blocks_count;
			long next_idx = first_idx+1;

			for(int i=0;i<block_count;++i)
			{
				node &n = b->nodes[i];
				n.block = -1;
				n.next_free = next_idx;
				++next_idx;
			}

			b->nodes[block_count-1].next_free = m_free_count;
			m_free_count = first_idx;

			m_blocks.push_back(b);

			free_block = prev_blocks_count;
			free_offset=0;
		}
		else
		{
			free_block = m_free_count / block_count;
			free_offset = m_free_count % block_count;
		}

		node &n = m_blocks[free_block]->nodes[free_offset];
		m_free_count = n.next_free;

		n.block = free_block;
		n.next_free = -1;

		++m_used_count;

		new (n.data) t_data;
		return (t_data*)n.data;
	}

	bool free(t_data *data )
	{
		if (!data)
			return false;

		node t;
		const int offset = (int)((char*)&t.data - (char*)&t.block);
		node &n = *((node *)(((char *)data) - offset));

		if (n.block<0 || n.block>=(int)m_blocks.size())
			return false;

		const long node_idx = n.block*block_count + 
					(int)(&n - &(m_blocks[n.block]->nodes[0]));

		if (node_idx<0 || node_idx>=(int)m_blocks.size()*block_count)
			return false;

		data->~t_data();

		n.block = -1;
		n.next_free = m_free_count;

		m_free_count = node_idx;

		--m_used_count;

		return true;
	 }

	void clear()
	{
		m_free_count = -1;

		long node_idx = 0;

		for (long i = 0;i<(long)m_blocks.size();++i)
		{
			for(int j = 0;j<block_count;++j)
			{
				node & n = m_blocks[i]->nodes[j];

				n.block = -1;
				n.next_free = m_free_count;

				m_free_count = node_idx;

				++node_idx;
			}
		}

		m_used_count = 0;
	}

public:
	long get_count() const
	{
		return m_used_count;
	}

	long get_mem_size() const
	{
		return long(m_blocks.size()*sizeof(t_data)*block_count);
	}

public:
	pool():m_free_count(-1),m_used_count(0) {}

	~pool()
	{
		for (long i = 0; i<(long)m_blocks.size(); ++i)
			delete m_blocks[i];
	}

private:
	pool(const pool &);
	void operator = (const pool &);

private:
	struct node
	{
		long block;
		long next_free;
		char data[sizeof(t_data)];
	};

	struct block
	{
		node nodes[block_count];
	};

	long m_free_count;
	long m_used_count;

	std::vector<block*> m_blocks;
};

}

#endif
