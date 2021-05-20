#include <valarray>
#include <iostream>
#include <fstream>
#include <array>

template<typename T>
std::ostream& operator<< (std::ostream& os, std::valarray<T> va)
{
    for (auto&& i : va)
        os << i << " ";
    return os;
}

template<typename T, size_t sz>
static std::valarray<T> arrayToValarray(std::array<T, sz> arr)
{
    std::valarray<T> varr(sz);
    std::copy(arr.begin(), arr.end(), std::begin(varr));
    return varr;
}

template <typename T>
class Array4D
{
public:
    Array4D(const std::array<size_t,4>& p_strides) :
    data(p_strides[0]*p_strides[1]*p_strides[2]*p_strides[3]),
    strides(p_strides)
    {
    }
    const std::valarray<T>& getData() const
    {
        return data;
    }
    T get(const std::array<size_t, 4>& pos) const
    {
        size_t flat_pos = pos_to_flat(pos);
        return data[flat_pos];
    }
    T getFlat(size_t flat_pos) const
    {
        return data[flat_pos];
    }
    void set(const std::array<size_t, 4>& pos, T val)
    {
        size_t flat_pos = pos_to_flat(pos);
        data[flat_pos] = val;
    }
    std::slice_array<T> getLine(const std::array<size_t, 4>& start_pos, size_t axis_direction)
    {
        // The axis direction is the dimension in which to generate
        // the sliced ("line")
        if(axis_direction>3) throw 403; // Error, outside bounds, forbidden
        size_t flat_start_pos = pos_to_flat(start_pos);

        size_t size_axis_dir = strides[axis_direction];

        if(start_pos[axis_direction] != 0) throw 404;
        // Error, the line must start in the beginning of the specified dimension

        size_t skip_amount = 1; // Amount to skip for selected axis direction
        for(size_t curr_axis = axis_direction; curr_axis > 0; --curr_axis)
        {
            skip_amount *= strides[curr_axis];
        }
        return data[std::slice(flat_start_pos, size_axis_dir, skip_amount)];
    }
    std::indirect_array<T> getRegionIndirect(const std::array<size_t, 4>& start_pos, const std::array<size_t, 4>& sizes)
    {
        size_t flat_start_pos = pos_to_flat(std::array<size_t, 4>(start_pos));
        std::valarray<size_t> sizes_varr((arrayToValarray(sizes)));
        std::array<size_t, 4> gstrides = getGStrides();
        std::valarray<size_t> gstrides_varr(arrayToValarray(gstrides));
        /*std::cout << "flat_start_pos: " << flat_start_pos << std::endl;
        std::cout << "sizes_varr: " << sizes_varr << std::endl;
        std::cout << "gstrides_varr: " << gstrides_varr << std::endl;*/
        std::valarray<size_t> indices(sizes[3]*sizes[2]*sizes[1]*sizes[0]);
        for (size_t i=0;i<sizes[0]; i++)
        {
            for (size_t j=0;j<sizes[1]; j++)
            {
                for (size_t k=0;k<sizes[2]; k++)
                {
                    for (size_t l=0;l<sizes[3]; l++)
                    {
                        size_t indpos = pos_to_flat({l,k,j,i});
                        //std::cout << "indpos: " << indpos << std::endl;
                        size_t pos = flat_start_pos + i*gstrides[0] + j*gstrides[1] + k*gstrides[2] + l*gstrides[3];
                        //std::cout << "pos: " << pos << std::endl;
                        indices[indpos] = pos;
                        //std::cout << "value_associated: " << data[pos] << std::endl;
                    }
                }
            }
        }
        return data[indices];
    }
    std::gslice getRegion(const std::array<size_t, 4>& start_pos, const std::array<size_t, 4>& sizes) const
    {
        size_t flat_start_pos = pos_to_flat(std::array<size_t, 4>(start_pos));
        std::valarray<size_t> sizes_varr((arrayToValarray(sizes)));
        std::array<size_t, 4> gstrides = getGStrides();
        std::valarray<size_t> gstrides_varr(arrayToValarray(gstrides));
        /*std::cout << "flat_start_pos: " << flat_start_pos << std::endl;
        std::cout << "sizes_varr: " << sizes_varr << std::endl;
        std::cout << "gstrides_varr: " << gstrides_varr << std::endl;*/
        return std::gslice(flat_start_pos, sizes_varr, gstrides_varr);
    }
    inline size_t pos_to_flat(const std::array<size_t, 4>& pos) const
    {
        size_t flat_pos = 
          pos[0]*(strides[3]*strides[2]*strides[1])
        + pos[1]*(strides[3]*strides[2])
        + pos[2]*strides[3]
        + pos[3];
        return flat_pos;
    }
    std::array<size_t, 4> getGStrides() const
    {
        //return {1, strides[0], strides[1]*strides[0], strides[2]*strides[1]*strides[0]};
        //return {strides[2]*strides[1]*strides[0], strides[1]*strides[0], strides[0], 1};
        //return {1, strides[3], strides[3]*strides[2], strides[3]*strides[2]*strides[1]};
        return {strides[3]*strides[2]*strides[1], strides[3]*strides[2], strides[3], 1};
    }
    std::array<size_t, 4> getStrides() const
    {
        return strides;
    }
    size_t getTotalSize() const
    {
        return strides[0]*strides[1]*strides[2]*strides[3];
    }

	bool readFromFile(std::string fname)
	{
		std::ifstream file;
		file.open(fname.c_str(), std::ios::binary | std::ios::in);
		file.read(std::begin(data), getTotalSize() * sizeof(T));
		return true;
	}
	bool writeToFile(std::string fname) const
	{
		std::ofstream file;
		file.open(fname.c_str(), std::ios::binary | std::ios::out);
		if (!file.is_open())
		{
			return false;
		}
		file.write((char*)std::begin(data), getTotalSize() * sizeof(T));
		return true;
	}
    void fill(T value)
    {
        std::fill(std::begin(data), std::end(data), value);
    }
public:
    std::valarray<T> data;
    std::array<size_t, 4> strides;
};
