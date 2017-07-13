/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_DAEMON_INRPPFACE_HPP
#define NFD_DAEMON_INRPPFACE_HPP

#include "transport.hpp"
#include "link-service.hpp"
#include "face-counters.hpp"
#include "face-log.hpp"
#include "face.hpp"

namespace nfd {
namespace face {

typedef TransportState FaceState;

/** \brief generalization of a network interface
 *
 *  A face generalizes a network interface.
 *  It provides best-effort network-layer packet delivery services
 *  on a physical interface, an overlay tunnel, or a link to a local application.
 *
 *  A face combines two parts: LinkService and Transport.
 *  Transport is the lower part, which provides best-effort TLV block deliveries.
 *  LinkService is the upper part, which translates between network-layer packets
 *  and TLV blocks, and may provide additional services such as fragmentation and reassembly.
 */
class InrppFace
#ifndef WITH_TESTS
final
#endif
  : public enable_shared_from_this<InrppFace>, noncopyable
{
public:
  InrppFace(unique_ptr<LinkService> service, unique_ptr<Transport> transport);

  LinkService*
  getLinkService() const;

  Transport*
  getTransport() const;

public: // upper interface connected to forwarding
  /** \brief sends Interest on Face
   */
  void
  sendInterest(const Interest& interest);

  /** \brief sends Data on Face
   */
  void
  sendData(const Data& data);

  /** \brief sends Nack on Face
   */
  void
  sendNack(const lp::Nack& nack);

  /** \brief signals on Interest received
   */
  signal::Signal<LinkService, Interest>& afterReceiveInterest;

  /** \brief signals on Data received
   */
  signal::Signal<LinkService, Data>& afterReceiveData;

  /** \brief signals on Nack received
   */
  signal::Signal<LinkService, lp::Nack>& afterReceiveNack;

public: // static properties
  /** \return face ID
   */
  FaceId
  getId() const;

  /** \brief sets face ID
   *  \note Normally, this should only be invoked by FaceTable.
   */
  void
  setId(FaceId id);

  void
  setMetric(uint64_t metric);

  uint64_t
  getMetric() const;

  /** \return a FaceUri representing local endpoint
   */
  FaceUri
  getLocalUri() const;

  /** \return a FaceUri representing remote endpoint
   */
  FaceUri
  getRemoteUri() const;

  /** \return whether face is local or non-local for scope control purpose
   */
  ndn::nfd::FaceScope
  getScope() const;

  /** \return face persistency setting
   */
  ndn::nfd::FacePersistency
  getPersistency() const;

  /** \brief changes face persistency setting
   */
  void
  setPersistency(ndn::nfd::FacePersistency persistency);

  /** \return whether face is point-to-point or multi-access
   */
  ndn::nfd::LinkType
  getLinkType() const;

public: // dynamic properties
  /** \return face state
   */
  FaceState
  getState() const;

  /** \brief signals after face state changed
   */
  signal::Signal<Transport, FaceState/*old*/, FaceState/*new*/>& afterStateChange;

  /** \return expiration time of the face
   *  \retval time::steady_clock::TimePoint::max() the face has an indefinite lifetime
   */
  time::steady_clock::TimePoint
  getExpirationTime() const;

  /** \brief request the face to be closed
   *
   *  This operation is effective only if face is in UP or DOWN state,
   *  otherwise it has no effect.
   *  The face changes state to CLOSING, and performs cleanup procedure.
   *  The state will be changed to CLOSED when cleanup is complete, which may
   *  happen synchronously or asynchronously.
   *
   *  \warning the face must not be deallocated until its state changes to CLOSED
   */
  void
  close();

  const FaceCounters&
  getCounters() const;

private:
  FaceId m_id;
  unique_ptr<LinkService> m_service;
  unique_ptr<Transport> m_transport;
  FaceCounters m_counters;
  uint64_t m_metric;
};

inline LinkService*
InrppFace::getLinkService() const
{
  return m_service.get();
}

inline Transport*
InrppFace::getTransport() const
{
  return m_transport.get();
}

inline void
InrppFace::sendInterest(const Interest& interest)
{
  m_service->sendInterest(interest);
}

inline void
InrppFace::sendData(const Data& data)
{
  m_service->sendData(data);
}

inline void
InrppFace::sendNack(const lp::Nack& nack)
{
  m_service->sendNack(nack);
}

inline FaceId
InrppFace::getId() const
{
  return m_id;
}

inline void
InrppFace::setId(FaceId id)
{
  m_id = id;
}

inline void
InrppFace::setMetric(uint64_t metric)
{
  m_metric = metric;
}

inline uint64_t
InrppFace::getMetric() const
{
  return m_metric;
}

inline FaceUri
InrppFace::getLocalUri() const
{
  return m_transport->getLocalUri();
}

inline FaceUri
InrppFace::getRemoteUri() const
{
  return m_transport->getRemoteUri();
}

inline ndn::nfd::FaceScope
InrppFace::getScope() const
{
  return m_transport->getScope();
}

inline ndn::nfd::FacePersistency
InrppFace::getPersistency() const
{
  return m_transport->getPersistency();
}

inline void
InrppFace::setPersistency(ndn::nfd::FacePersistency persistency)
{
  return m_transport->setPersistency(persistency);
}

inline ndn::nfd::LinkType
InrppFace::getLinkType() const
{
  return m_transport->getLinkType();
}

inline FaceState
InrppFace::getState() const
{
  return m_transport->getState();
}

inline time::steady_clock::TimePoint
InrppFace::getExpirationTime() const
{
  return m_transport->getExpirationTime();
}

inline void
InrppFace::close()
{
  m_transport->close();
}

inline const FaceCounters&
InrppFace::getCounters() const
{
  return m_counters;
}

std::ostream&
operator<<(std::ostream& os, const FaceLogHelper<InrppFace>& flh);

template<typename T>
typename std::enable_if<std::is_base_of<InrppFace, T>::value, std::ostream&>::type
operator<<(std::ostream& os, const FaceLogHelper<T>& flh)
{
  return os << FaceLogHelper<InrppFace>(flh.obj);
}

} // namespace face

using face::FaceId;
using face::Face;

} // namespace nfd

#endif // NFD_DAEMON_FACE_HPP