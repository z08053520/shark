/*!
 *  \brief implements the absolute loss, which is the distance between labels and predictions
 *
 *
 *  \author  Tobias Glasmachers
 *  \date    2011
 *
 *  \par Copyright (c) 1999-2011:
 *      Institut f&uuml;r Neuroinformatik<BR>
 *      Ruhr-Universit&auml;t Bochum<BR>
 *      D-44780 Bochum, Germany<BR>
 *      Phone: +49-234-32-25558<BR>
 *      Fax:   +49-234-32-14209<BR>
 *      eMail: Shark-admin@neuroinformatik.ruhr-uni-bochum.de<BR>
 *      www:   http://www.neuroinformatik.ruhr-uni-bochum.de<BR>
 *
 *
 *  <BR><HR>
 *  This file is part of Shark. This library is free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software
 *  Foundation; either version 3, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef SHARK_OBJECTIVEFUNCTIONS_LOSS_ABSOLUTELOSS_H
#define SHARK_OBJECTIVEFUNCTIONS_LOSS_ABSOLUTELOSS_H


#include <shark/ObjectiveFunctions/Loss/AbstractLoss.h>
namespace shark{


///
/// \brief absolute loss
///
/// The absolute loss is usually defined in a single dimension
/// as the absolute value of the difference between labels and
/// predictions. Here we generalize to multiple dimensions by
/// returning the norm.
///
template<class VectorType = RealVector>
class AbsoluteLoss : public AbstractLoss<VectorType, VectorType>
{
public:
	typedef AbstractLoss<VectorType, VectorType> base_type;
	typedef typename base_type::BatchLabelType BatchLabelType;
	typedef typename base_type::BatchOutputType BatchOutputType;

	/// constructor
	AbsoluteLoss()
	{ }


	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "AbsoluteLoss"; }

	// annoyingness of C++ templates
	using base_type::eval;

	/// evaluate the loss \f$ \| labels - predictions \| \f$, which
	/// is a slight generalization of the absolute value of the difference.
	double eval(BatchLabelType const& labels, BatchOutputType const& predictions) const{
		SIZE_CHECK(labels.size1() == predictions.size1());
		SIZE_CHECK(labels.size2() == predictions.size2());

		double error = 0;
		for(std::size_t i = 0; i != labels.size1(); ++i){
			error+=blas::distance(row(predictions,i),row(labels,i));
		}
		return error;
	}
};


}
#endif
