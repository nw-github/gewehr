struct matrix4x4_t
{
	float data[4][4];

	float* operator[](size_t index)
	{
		return data[index];
	}

	const float* operator[](size_t index) const
	{
		return data[index];
	}
};

struct matrix3x4_t
{
	float data[3][4];

	float* operator[](size_t index)
	{
		return data[index];
	}

	const float* operator[](size_t index) const
	{
		return data[index];
	}
};
