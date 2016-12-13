/**
@file    wb_traj_generator.cpp
@author  Alexander Winkler (winklera@ethz.ch)
@date    Oct 21, 2016
@brief   Defines the class WholeBody Trajectory Generator
 */

#ifndef _XPP_XPP_OPT_WB_TRAJ_GENERATOR_H_
#define _XPP_XPP_OPT_WB_TRAJ_GENERATOR_H_

#include "com_motion.h"
#include "motion_phase.h"
#include "wb_traj_in_out.h"

#include <xpp/utils/polynomial_helpers.h>
#include <xpp/utils/polynomial_xd.h>
#include <xpp/utils/eigen_std_conversions.h>

namespace xpp {
namespace opt {


/** @brief Whole-Body Trajectory Generator
  *
  * This class is responsible for taking the optimized trajectory and
  * filling in the remaining DoF to produce a discretized whole body trajectory.
  * The DoF that are calculated by this class include:
  *   - Body height
  *   - Angular pos/vel/acc
  *   - Swingleg trajectories.
  */
template<size_t N_EE>
class WBTrajGenerator {
public:
  using ComMotionS    = std::shared_ptr<xpp::opt::ComMotion>;
  using Vector3d      = Eigen::Vector3d;
  using VecFoothold   = utils::StdVecEigen2d;
  using State3d       = xpp::utils::StateLin3d;
  using SplinerOri    = xpp::utils::PolynomialXd< utils::CubicPolynomial, State3d>;
  using SplinerFeet   = xpp::utils::PolynomialXd< utils::QuinticPolynomial, State3d>;
  using ZPolynomial   = xpp::utils::CubicPolynomial;
  using PhaseVec      = std::vector<MotionPhase>;

  using SplineNode    = Node<N_EE>;
  using FeetArray     = typename SplineNode::FeetArray;
  using ContactArray  = typename SplineNode::ContactArray;
  using ArtiRobVec    = std::vector<ArticulatedRobotState<N_EE> >;
  using FeetSplinerArray = std::array<SplinerFeet, N_EE>;

public:
  WBTrajGenerator();
  virtual ~WBTrajGenerator();

  void SetParams(double upswing, double lift_height,
                 double outward_swing_distance,
                 double discretization_time);

  void Init(const PhaseVec&,
            const ComMotionS&,
            const VecFoothold&,
            double des_height,
            const SplineNode& curr_state);

  ArtiRobVec BuildWholeBodyTrajectory() const;

private:
  std::vector<SplineNode> nodes_; // the discrete states to spline through
  std::vector<ZPolynomial> z_spliner_;
  std::vector<SplinerOri> ori_spliner_;
  std::vector<FeetSplinerArray> feet_spliner_up_, feet_spliner_down_;
  ComMotionS com_motion_;

  double kDiscretizationTime;   // at what interval the continuous trajectory is sampled
  double kUpswingPercent;       // how long to swing up during swing
  double kLiftHeight;           // how high to lift the leg
  double kOutwardSwingDistance; // how far to swing leg outward (y-dir)

  std::vector<SplineNode> BuildNodeSequence(const SplineNode& P_init,
                                            const PhaseVec&,
                                            const VecFoothold& footholds,
                                            double des_robot_height);

  void CreateAllSplines(const std::vector<SplineNode>& nodes);

  State3d GetCurrPosition(double t_global) const;
  xpp::utils::StateAng3d GetCurrOrientation(double t_global) const;
  FeetArray GetCurrEndeffectors(double t_global) const;
  ContactArray GetCurrContactState(double t_gloal) const;

  void FillZState(double t_global, State3d& pos) const;

  SplinerOri BuildOrientationRpySpline(const SplineNode& from, const SplineNode& to) const;
  FeetSplinerArray BuildFootstepSplineUp(const SplineNode& from, const SplineNode& to) const;
  FeetSplinerArray BuildFootstepSplineDown(const FeetArray& feet_at_switch,const SplineNode& to) const;

  void BuildOneSegment(const SplineNode& from, const SplineNode& to,
                       ZPolynomial& z_poly,
                       SplinerOri& ori,
                       FeetSplinerArray& feet_up,
                       FeetSplinerArray& feet_down) const;

  static Vector3d TransformQuatToRpy(const Eigen::Quaterniond& q);
  int GetSplineID(double t_global) const;
  double GetLocalSplineTime(double t_global) const;
  double GetTotalTime() const;
};

} // namespace opt
} // namespace xpp

#include "implementation/wb_traj_generator-impl.h"

#endif // _XPP_XPP_OPT_WB_TRAJ_GENERATOR_H_
