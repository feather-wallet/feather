// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) The Monero Project.

#ifndef FEATHER_BOUNTY_H
#define FEATHER_BOUNTY_H

#include <QString>
#include <utility>

struct BountyEntry {
    BountyEntry(int votes, QString title, double bountyAmount, QString link, QString donationAddress, QString status)
        : votes(votes), title(std::move(title)), bountyAmount(bountyAmount), link(std::move(link)),
        donationAddress(std::move(donationAddress)), status(std::move(status)){};

    int votes;
    QString title;
    double bountyAmount;
    QString link;
    QString donationAddress;
    QString status;
};

#endif //FEATHER_BOUNTY_H
