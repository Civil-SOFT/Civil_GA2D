/***
 * CivilMatrix.h;
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Eng.� Anderson Marques Ribeiro (anderson.marques.ribeiro@gmail.com).
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __CIVIL_MATRIX
#define __CIVIL_MATRIX

#ifdef _MANAGED
#pragma unmanaged
#endif // ifdef _MANAGED

#include "..\UtilsLibrary\CivilError.h"
#include "..\UtilsLibrary\CivilRange.h"
#include "..\UtilsLibrary\CivilDynArray.h"

using namespace CIVIL::UTILS;

namespace CIVIL::MATH::GA2D
{

#define MAX_ROWS 20
#define MAX_COLS 20

	DECLARE_ERROR_CODE(meInvalidIndex);
	DECLARE_ERROR_CODE(meCantRemoveDim);
	DECLARE_ERROR_CODE(meInvertible);
	DECLARE_ERROR_CODE(meNotSquare);
	DECLARE_ERROR_CODE(meIncompatible);

	BEGIN_DECLARE_ERROR(EMatrix)
		DECLARE_ERROR(meInvalidIndex, "Invalid index")
		DECLARE_ERROR(meCantRemoveDim, "Row/colunm can not be removed")
		DECLARE_ERROR(meInvertible, "impossible to calculate the inverse matrix")
		DECLARE_ERROR(meNotSquare, "Invalid operation for non-square matrix")
		DECLARE_ERROR(meIncompatible, "Matrices incompatible for operation")
	END_DECLARE_ERROR;

	template<typename _type = double>
	struct Matrix
	{
	public:

		typedef Range<short int, 0, SHRT_MAX - 1> RowRangeType, ColRangeType;
		typedef Range<short int, 1, SHRT_MAX> RowSizeType, ColSizeType;

		Matrix()
			: m_aItems(0, 0)
		{}
		Matrix(const RowSizeType &rows, const ColSizeType &cols)
			: m_aItems(DynArray<_type>(rows, cols))
		{}
		Matrix(const Matrix &mat)
		{
			*this = mat;
		}
		Matrix(_type *values, const RowSizeType &rows, const ColSizeType &cols) :
			m_aItems(DynArray<_type>(rows, cols))
		{
			for (int i = 0; i < rows; i++)
				for (int j = 0; j < cols; j++)
					m_aItems.setItem(i, j, values[i * cols + j]);
		}

	private:

		DynArray<_type>
			m_aItems;

	public:

		RowSizeType getRowCount() const
		{
			return m_aItems.getRowCount();
		}
		ColSizeType getColCount() const
		{
			return m_aItems.getColCount();
		}
		void setDims(const RowSizeType &rows, const ColSizeType &cols)
		{
			m_aItems.setDims(rows, cols);
		}

		_type getItem(const RowRangeType &row, const ColRangeType &col) const
		{
			return m_aItems.getItem(row, col);
		}
		void setItem(const RowRangeType &row, const ColRangeType &col, _type value)
		{
			m_aItems.setItem(row, col, value);
		}

		void removeRow(const RowRangeType &row)
		{
			if (row >= getRowCount())
				RAISE(EMatrix, meInvalidIndex);

			if (getRowCount() == 1)
				RAISE(EMatrix, meCantRemoveDim);

			for (int i = row; i < m_aItems.getRowCount() - 1; i++)
				for (int j = 0; j < m_aItems.getColCount(); j++)
					m_aItems.setItem(i, j, m_aItems.getItem(i + 1, j));

			m_aItems.setDims(m_aItems.getRowCount() - 1, m_aItems.getColCount());
		}
		void removeCol(const ColRangeType &col)
		{
			if (col >= getColCount())
				RAISE(EMatrix, meInvalidIndex);

			if (getColCount() == 1)
				RAISE(EMatrix, meCantRemoveDim);

			for (int i = 0; i < m_aItems.getRowCount(); i++)
				for (int j = col; j < m_aItems.getColCount() - 1; j++)
					m_aItems.setItem(i, j, m_aItems.getItem(i, j + 1));
			m_aItems.setDims(m_aItems.getRowCount(), m_aItems.getColCount() - 1);
		}

		static Matrix null(const RowSizeType &rows, const ColSizeType &cols)
		{
			Matrix
				mat(rows, cols);

			for (int i = 0; i < rows; i++)
				for (int j = 0; j < cols; j++)
					mat.setItem(i, j, 0);

			return mat;
		}
		static Matrix identity(const RowSizeType &size)
		{
			Matrix
				mat(size, size);

			for (int i = 0; i < size; i++)
				for (int j = 0; j < size; j++)
					mat.setItem(i, j, i == j ? 1 : 0);

			return mat;
		}

		bool isSquare() const
		{
			return getRowCount() == getColCount();
		}

		Matrix cofats()
		{
			if (!isSquare())
				RAISE(EMatrix, meNotSquare);

			Matrix
				tmp,
				res(getRowCount(), getColCount());

			for (int i = 0; i < getRowCount(); i++)
				for (int j = 0; j < getColCount(); j++)
				{
					tmp = *this;
					tmp.removeRow(i);
					tmp.removeCol(j);

					res.setItem(i, j, (_type)pow(-1, (i + 1) + (j + 1)) * tmp.calcDet());
				}

			return res;
		}

		Matrix transposed()
		{
			Matrix
				res(getColCount(), getRowCount());

			for (int i = 0; i < getRowCount(); i++)
				for (int j = 0; j < getColCount(); j++)
					res.setItem(j, i, getItem(i, j));

			return res;
		}

		bool invertible()
		{
			return isSquare() && calcDet() != 0;
		}
		Matrix reverse()
		{
			if (!invertible())
				RAISE(EMatrix, meInvertible);

			return cofats().transposed() * (1 / calcDet());
		}

		Matrix primaryDiagonal() const
		{
			if (getRowCount() != getColCount())
				RAISE(EMatrix, meNotSquare);

			Matrix
				res(1, getRowCount());

			for (int i = 0; i < getRowCount(); i++)
				res.setItem(0, i, getItem(i, i));

			return res;
		}
		Matrix secundaryDiagonal() const
		{
			if (getRowCount() != getColCount())
				RAISE(EMatrix, meNotSquare);

			Matrix
				res(1, getRowCount());
			int
				j = getRowCount() - 1;

			for (int i = 0; i < getRowCount(); i++)
			{
				res.setItem(0, j, getItem(j, i));
				j--;
			}

			return res;
		}

		Matrix &operator=(const Matrix &mat)
		{
			m_aItems = mat.m_aItems;

			return *this;
		}

		friend Matrix operator+(const Matrix &mat1, const Matrix &mat2)
		{
			if (mat1.getRowCount() != mat2.getRowCount() || mat1.getColCount() != mat2.getColCount())
				RAISE(EMatrix, meIncompatible);

			Matrix
				res(mat1.getRowCount(), mat1.getColCount());

			for (int i = 0; i < getRowCount(); i++)
				for (int j = 0; j < getColCount(); j++)
					res.setItem(i, j, mat1.getItem(i, j) + mat2.getItem(i, j));

			return res;
		}
		friend Matrix operator+(const Matrix &mat, _type value)
		{
			Matrix
				res(mat.getRowCount(), mat.getColCount());

			for (int i = 0; i < mat.getRowCount(); i++)
				for (int j = 0; j < mat.getColCount(); j++)
					res.setItem(i, j, mat.getItem(i, j) + value);

			return res;
		}
		friend Matrix operator+(_type value, const Matrix &mat)
		{
			return operator+(mat, value);
		}
		Matrix &operator+=(const Matrix &mat)
		{
			*this = *this + mat;

			return *this;
		}
		Matrix &operator+=(_type value)
		{
			*this = *this + value;

			return *this;
		}

		friend Matrix operator-(const Matrix &mat1, const Matrix &mat2)
		{
			if (mat1.getRowCount() != mat2.getRowCount() || mat1.getColCount() != mat2.getColCount())
				RAISE(EMatrix, meIncompatible);

			Matrix
				res(mat1.getRowCount(), mat1.getColCount());

			for (int i = 0; i < res.getRowCount(); i++)
				for (int j = 0; j < res.getColCount(); j++)
					res.setItem(i, j, mat1.getItem(i, j) - mat2.getItem(i, j));

			return res;
		}
		friend Matrix operator-(const Matrix &mat, _type value)
		{
			Matrix
				res(mat.getRowCount(), mat.getColCount());

			for (int i = 0; i < res.getRowCount(); i++)
				for (int j = 0; j < res.getColCount(); j++)
					res.setItem(i, j, mat.getItem(i, j) - value);

			return res;
		}
		friend Matrix operator-(_type value, const Matrix &mat)
		{
			return operator-(mat, value);
		}
		Matrix &operator-=(const Matrix &mat)
		{
			*this = *this - mat;

			return *this;
		}
		Matrix &operator-=(_type value)
		{
			*this = *this - value;

			return *this;
		}

		friend Matrix operator*(const Matrix &mat1, const Matrix &mat2)
		{
			Matrix
				res(mat1.getRowCount(), mat2.getColCount());

			for (int i = 0; i < res.getRowCount(); i++)
				for (int j = 0; j < res.getColCount(); j++)
				{
					_type
						item = 0;

					for (int k = 0; k < mat1.getColCount(); k++)
						item += mat1.getItem(i, k) * mat2.getItem(k, j);

					res.setItem(i, j, item);
				}

			return res;
		}
		friend Matrix operator*(const Matrix &mat, _type value)
		{
			Matrix
				res(mat.getRowCount(), mat.getColCount());

			for (int i = 0; i < mat.getRowCount(); i++)
				for (int j = 0; j < mat.getColCount(); j++)
					res.setItem(i, j, mat.getItem(i, j) * value);

			return res;
		}
		friend Matrix operator*(_type value, const Matrix &mat)
		{
			return operator*(mat, value);
		}
		Matrix &operator*=(const Matrix &mat)
		{
			*this = *this * mat;

			return *this;
		}
		Matrix &operator*=(_type value)
		{
			*this = *this * value;

			return *this;
		}

		friend Matrix operator/(const Matrix &mat1, const Matrix &mat2)
		{
			return mat1 * mat2.reverse();
		}
		friend Matrix operator/(const Matrix &mat, _type value)
		{
			Matrix
				res(getRowCount(), getColCount());

			for (int i = 0; i < getRowCont(); i++)
				for (int j = 0; j < getColCount(); j++)
					res.setItem(i, j, mat.getItem(i, j) / value);

			return res;
		}
		friend Matrix operator/(_type value, const Matrix &mat)
		{
			return operator/(mat, value);
		}
		Matrix &operator/=(const Matrix &mat)
		{
			*this = *this / mat;

			return *this;
		}
		Matrix &operator/=(_type value)
		{
			*this = *this / value;

			return *this;
		}

	private:
			
		_type calcDet(const Matrix &mat)
		{
			_type
				det;
			Matrix
				matTemp;

			det = 0;

			switch (mat.getRowCount())
			{
			case 1:
				return mat.getItem(0, 0);
			case 2:
				return mat.getItem(0, 0) * mat.getItem(1, 1) - mat.getItem(1, 0) * mat.getItem(0, 1);
			default:
				for (int i = 0; i < mat.getColCount(); i++)
				{
					matTemp = mat;
					matTemp.removeRow(0);
					matTemp.removeCol(i);
					det += mat.getItem(0, i) * (_type) pow(-1, i) * calcDet(matTemp);
				}
			}

			return det;
		}

	public:

		_type calcDet()
		{
			return calcDet(*this);
		}

	}; /* Matrix */

} // namespace CIVIL::MATH::GA2D

#endif // ifndef __CIVIL_MATRIX
