/*	
    IFDocument.h
	Copyright 2002, Apple, Inc. All rights reserved.
*/

#include <Cocoa/Cocoa.h>

@class IFWebDataSource;
@class IFError;

@protocol IFDocumentView <NSObject>
- (void)provisionalDataSourceChanged:(IFWebDataSource *)dataSource;
- (void)provisionalDataSourceCommitted:(IFWebDataSource *)dataSource;
- (void)dataSourceUpdated:(IFWebDataSource *)dataSource; 
- (void)layout;
@end

@protocol IFDocumentDragSettings
- (void)setCanDragFrom: (BOOL)flag;
- (BOOL)canDragFrom;
- (void)setCanDragTo: (BOOL)flag;
- (BOOL)canDragTo;
@end

@protocol IFDocumentSearching
- (void)searchFor: (NSString *)string direction: (BOOL)forward caseSensitive: (BOOL)caseFlag;
@end

@protocol IFDocumentRepresentation <NSObject>
- (void)receivedData:(NSData *)data withDataSource:(IFWebDataSource *)dataSource;
- (void)receivedError:(IFError *)error withDataSource:(IFWebDataSource *)dataSource;
- (void)finishedLoadingWithDataSource:(IFWebDataSource *)dataSource;
@end