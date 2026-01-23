// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "macos_sleep.h"

#if defined(Q_OS_MACOS)

#import <Cocoa/Cocoa.h>

@interface FeatherSleepObserver : NSObject
@property (nonatomic, assign) MacSleepObserver *owner;
@end

@implementation FeatherSleepObserver
- (instancetype)initWithOwner:(MacSleepObserver *)owner {
    self = [super init];
    if (self) {
        _owner = owner;
        NSNotificationCenter *center = [[NSWorkspace sharedWorkspace] notificationCenter];
        [center addObserver:self selector:@selector(onWillSleep:) name:NSWorkspaceWillSleepNotification object:nil];
        [center addObserver:self selector:@selector(onDidWake:) name:NSWorkspaceDidWakeNotification object:nil];
    }
    return self;
}

- (void)dealloc {
    NSNotificationCenter *center = [[NSWorkspace sharedWorkspace] notificationCenter];
    [center removeObserver:self];
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

- (void)onWillSleep:(NSNotification *)__unused notification {
    if (_owner) {
        _owner->notifyWillSleep();
    }
}

- (void)onDidWake:(NSNotification *)__unused notification {
    if (_owner) {
        _owner->notifyDidWake();
    }
}
@end

MacSleepObserver::MacSleepObserver(QObject *parent)
    : QObject(parent)
{
    FeatherSleepObserver *observer = [[FeatherSleepObserver alloc] initWithOwner:this];
#if __has_feature(objc_arc)
    m_observer = (__bridge_retained void *)observer;
#else
    m_observer = observer;
#endif
}

MacSleepObserver::~MacSleepObserver() {
    if (m_observer) {
#if !__has_feature(objc_arc)
        FeatherSleepObserver *observer = static_cast<FeatherSleepObserver *>(m_observer);
        [observer release];
#else
        CFBridgingRelease(m_observer);
#endif
        m_observer = nullptr;
    }
}

void MacSleepObserver::notifyWillSleep() {
    emit willSleep();
}

void MacSleepObserver::notifyDidWake() {
    emit didWake();
}

#else

MacSleepObserver::MacSleepObserver(QObject *parent)
    : QObject(parent)
{}

MacSleepObserver::~MacSleepObserver() = default;

void MacSleepObserver::notifyWillSleep() {}

void MacSleepObserver::notifyDidWake() {}

#endif
