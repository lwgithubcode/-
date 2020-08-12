#include"bigInt.h"

void bigint::LoadData(int sz)
{
	srand((unsigned int)time(0));
	for (int i = 1; i <= sz; i++)
	{
		u_char ch = rand() % 10;
		big.push_back(rand() % 10);
	}
	this->clear_head_zero();
}

void bigint::ShowData()
{
	for (size_t i = size(); i > 0; --i)
		cout << (int)big[i];
	cout << endl;
}

bigint& bigint::operator=(int num)
{
	this->clear();
	this->push_back(0);
	return *this;
}


void bigint::push_back(u_char value)
{
	big.push_back(value);
}

void bigint::push_front(u_char value)
{
	big.push_front(value);
}


size_t bigint::size()
{
	return big.size();
}

void bigint::clear()
{
	this->big.clear();
}




void bigint::Add(bigint & bt, bigint& bt1, bigint& bt2)
{
	size_t index = 1;
	u_char pro = 0;
	bt.clear();
	while (index <= bt1.size() && index <= bt2.size())
	{
		pro += bt1[index] + bt2[index];
		bt.push_back(pro % 10);
		pro /= 10;
		++index;
	}
	while (index <= bt1.size())
	{
		pro += bt1[index];
		bt.push_back(pro % 10);
		pro /= 10;
		++index;
	}
	while (index <= bt2.size())
	{
		pro += bt2[index];
		bt.push_back(pro % 10);
		pro /= 10;
		++index;
	}
	if (pro != 0)
		bt.push_back(pro);
}

void bigint::Sub(bigint & bt, bigint& bt1, bigint& bt2)
{
	if (bt1 < bt2)
		return;
	else if (bt1 == bt2)
		bt = 0;
	else
	{
		bt.clear();
		u_char sign = 0;
		size_t index = 1;
		while(index <= bt2.size())
		{
			int value = 0;
			if (bt1[index] < sign || bt1[index] - sign < bt2[index])
			{
				value = 10 + bt1[index] - sign;
				sign = 1;
			}
			else
			{
				value = bt1[index] - sign;
				sign = 0;
			}
			bt.push_back(value - bt2[index]);
			index++;
		}
		while (sign != 0 && index <= bt1.size())
		{
			int value = 0;
			if (bt1[index] < sign)
			{
				value = 10 + bt1[index] - sign;
				sign = 1;
			}
			else
			{
				value = bt1[index] - sign;
				sign = 0;
			}
			bt.push_back(value);
		}
		bt.clear_head_zero();
	}
}


void bigint::Mul(bigint & bt, bigint& bt1, bigint& bt2)
{
	bigint result[10];
	for (int i = 1; i <= 9; i++)
		Mul(result[i], bt1, i);
	bt.clear();
	for (size_t i = 1; i <= bt2.size(); i++)
	{
		if (bt2[i] == 0)
			continue;
		bigint temp;
		Mul(temp, bt1, bt2[i]);
		temp.Move(i - 1);
		bt += temp;
	}
}


void bigint::Div(bigint & bt, bigint& bt1, bigint& bt2)
{
	bt.clear();
	if (bt1 < bt2)
		bt.push_back(0);
	else if (bt1 == bt2)
		bt.push_back(1);
	else
	{
		int start = bt1.size() - bt2.size();
		bigint temp;
		temp.clear();
		for (size_t i = 1; i <= bt2.size(); i++)
			temp.push_back(bt1[i + start]);
		while (start >= 0)
		{
			u_char div = 0;
			while (temp >= bt2)
			{
				temp -= bt2;
				div++;
				temp.clear_head_zero();
			}
			bt.push_front(div);
			temp.push_front(bt1[start]);
			start--;
		}
		bt.clear_head_zero();
	}
}

void bigint::clear_head_zero()
{
	while ((*this)[size()] == 0)
	{
		(*this)[size()] = u_char();
		this->big.size()--;
	}
}


void bigint::Mul(bigint & bt, bigint& bt1, int x)
{
	int signal = 0;
	size_t index = 1;
	while (index <= bt1.size())
	{
		signal += bt1[index] * x;
		bt.push_back(signal % 10);
		signal /= 10;
		++index;
	}
	if (signal != 0)
		bt.push_back(signal);
}

void bigint::Move(int x)
{
	for (int i = 1; i <= x; i++)
		this->push_front(0);
}

void bigint::operator+=(bigint& bt)
{
	u_char signal = 0;
	size_t index = 1;
	while (index <= this->size() && index <= bt.size())
	{
		signal += (*this)[index] + bt[index];
		(*this)[index] = signal % 10;
		signal /= 10;
		index++;
	}
	while (signal != 0 && index <= this->size())
	{
		signal += (*this)[index];
		(*this)[index] = signal % 10;
		signal /= 10;
		index++;
	}
	while (index <= bt.size())
	{
		signal += bt[index];
		(*this).push_back(signal % 10);
		signal /= 10;
		index++;
	}
	if (signal != 0)
		this->push_back(signal);
}

void bigint::operator-=(bigint& bt)
{
	if (*this <= bt)
	{
		this->clear();
		this->push_back(0);
	}
	else
	{
		int signal = 0;
		size_t index = 1;
		while (index <= bt.size())
		{
			u_char value = 0;
			if (signal == 1 && (*this)[index] == 0 || (*this)[index] - signal < bt[index])
			{
				value = 10 + (*this)[index] - signal;
				signal = 1;
			}
			else
			{
				value = (*this)[index] - signal;
				signal = 0;
			}
			(*this)[index] = value - bt[index];
			index++;
		}
		while (index <= this->size())
		{
			if ((*this)[index] == 0)
			{
				(*this)[index] = 10 - signal;
				signal = 1;
			}
			else
			{
				(*this)[index] -= signal;
				signal = 0;
			}
			++index;
		}
	}
}


u_char& bigint::operator[](int index)
{
	return big[index];
}




bool bigint::operator<(bigint& bt)
{
	return compare(bt) == -1;
}
bool bigint::operator<=(bigint& bt)
{
	return compare(bt) <= 0;
}
bool bigint::operator>(bigint& bt)
{
	return compare(bt) == 1;
}
bool bigint::operator>=(bigint& bt)
{
	return compare(bt) >= 0;
}
bool bigint::operator==(bigint& bt)
{
	return compare(bt) == 0;
}
bool bigint::operator!=(bigint& bt)
{
	return compare(bt) != 0;
}

int bigint::compare(bigint& bt)
{
	if (this->size() > bt.size())
		return 1;
	else if (this->size() < bt.size())
		return -1;
	else
	{
		for (size_t i = bt.size(); i >= 1; i--)
		{
			if ((*this)[i] > bt[i])
				return 1;
			else if ((*this)[i] < bt[i])
				return -1;
		}
		return 0;
	}
}
