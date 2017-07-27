// AFImageDownloader.h
// Copyright (c) 2011–2016 Alamofire Software Foundation ( http://alamofire.org/ )
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//系统版本信息
#import <TargetConditionals.h>

#if TARGET_OS_IOS || TARGET_OS_TV 

#import <Foundation/Foundation.h>
//图片缓存管理
#import "AFAutoPurgingImageCache.h"
//http会话管理
#import "AFHTTPSessionManager.h"

NS_ASSUME_NONNULL_BEGIN

//图片下载队列方式，先入先出或后入先出
typedef NS_ENUM(NSInteger, AFImageDownloadPrioritization) {
    AFImageDownloadPrioritizationFIFO,
    AFImageDownloadPrioritizationLIFO
};

/**
 The `AFImageDownloadReceipt` is an object vended by the `AFImageDownloader` when starting a data task. It can be used to cancel active tasks running on the `AFImageDownloader` session. As a general rule, image data tasks should be cancelled using the `AFImageDownloadReceipt` instead of calling `cancel` directly on the `task` itself. The `AFImageDownloader` is optimized to handle duplicate task scenarios as well as pending versus active downloads.
 */
//当开始一个新任务的时候，AFImageDownloadReceipt（收据）被AFImageDownloader管理
//它能用来取消在AFImageDownloader中运行的会话。基于一般的规则，图片数据任务可以被AFImageDownloadReceipt取消
//而不需要直接调用任务的cancel函数。
//AFImageDownloader优化去掌握重复任务情节和挂起主动下载一样好
@interface AFImageDownloadReceipt : NSObject

/**
 The data task created by the `AFImageDownloader`.
*/
//AFImageDownloader生成的数据下载任务
@property (nonatomic, strong) NSURLSessionDataTask *task;

/**
 The unique identifier for the success and failure blocks when duplicate requests are made.
 */
//AFImageDownloadReceipt的唯一标识，用来调用成功或者失败回调
@property (nonatomic, strong) NSUUID *receiptID;
@end

/** The `AFImageDownloader` class is responsible for downloading images in parallel on a prioritized queue. Incoming downloads are added to the front or back of the queue depending on the download prioritization. Each downloaded image is cached in the underlying `NSURLCache` as well as the in-memory image cache. By default, any download request with a cached image equivalent in the image cache will automatically be served the cached image representation.
 */
//AFImageDownloader负责在优先队列中并行下载图片。
//根据优先级不同，新加入的下载会加入到队列头部或队列尾部。
//每个下载下来的图片都缓存在NSURLCache以及内存缓存中。
//任何与图像缓存中的缓存图像等效的下载请求都将自动缓存。
@interface AFImageDownloader : NSObject

/**
 The image cache used to store all downloaded images in. `AFAutoPurgingImageCache` by default.
 */
//图像缓存，缓存所有下载图像。默认为AFAutoPurgingImageCache对象
@property (nonatomic, strong, nullable) id <AFImageRequestCache> imageCache;

/**
 The `AFHTTPSessionManager` used to download images. By default, this is configured with an `AFImageResponseSerializer`, and a shared `NSURLCache` for all image downloads.
 */
//http会话管理，用来管理图像下载。默认使用AFImageResponseSerializer响应序列化，并且所有图像下载共享NSURLCache
@property (nonatomic, strong) AFHTTPSessionManager *sessionManager;

/**
 Defines the order prioritization of incoming download requests being inserted into the queue. `AFImageDownloadPrioritizationFIFO` by default.
 */
//设置新加入队列的下载请求插入队列的方式。默认先入先出方式
@property (nonatomic, assign) AFImageDownloadPrioritization downloadPrioritizaton;

/**
 The shared default instance of `AFImageDownloader` initialized with default values.
 */
//返回AFImageDownloader单例，使用默认数值初始化
+ (instancetype)defaultInstance;

/**
 Creates a default `NSURLCache` with common usage parameter values.

 @returns The default `NSURLCache` instance.
 */
//生成默认NSURLCache
+ (NSURLCache *)defaultURLCache;

/**
 Default initializer

 @return An instance of `AFImageDownloader` initialized with default values.
 */
//初始化对象
- (instancetype)init;

/**
 Initializes the `AFImageDownloader` instance with the given session manager, download prioritization, maximum active download count and image cache.

 @param sessionManager The session manager to use to download images.
 @param downloadPrioritization The download prioritization of the download queue.
 @param maximumActiveDownloads  The maximum number of active downloads allowed at any given time. Recommend `4`.
 @param imageCache The image cache used to store all downloaded images in.

 @return The new `AFImageDownloader` instance.
 */
//使用传入的会话管理对象，插入队列方式，最大下载数，图片缓存初始化对象
- (instancetype)initWithSessionManager:(AFHTTPSessionManager *)sessionManager
                downloadPrioritization:(AFImageDownloadPrioritization)downloadPrioritization
                maximumActiveDownloads:(NSInteger)maximumActiveDownloads
                            imageCache:(nullable id <AFImageRequestCache>)imageCache;

/**
 Creates a data task using the `sessionManager` instance for the specified URL request.

 If the same data task is already in the queue or currently being downloaded, the success and failure blocks are
 appended to the already existing task. Once the task completes, all success or failure blocks attached to the
 task are executed in the order they were added.

 @param request The URL request.
 @param success A block to be executed when the image data task finishes successfully. This block has no return value and takes three arguments: the request sent from the client, the response received from the server, and the image created from the response data of request. If the image was returned from cache, the response parameter will be `nil`.
 @param failure A block object to be executed when the image data task finishes unsuccessfully, or that finishes successfully. This block has no return value and takes three arguments: the request sent from the client, the response received from the server, and the error object describing the network or parsing error that occurred.

 @return The image download receipt for the data task if available. `nil` if the image is stored in the cache.
 cache and the URL request cache policy allows the cache to be used.
 */
//使用指定的会话管理对象为指定url请求创建数据任务
//如果相同的数据任务已经存在或者开始下载，成功或者失败的block会被加到已经存在的任务上
//当任务完成的时候，会触发所有的成功和失败block
- (nullable AFImageDownloadReceipt *)downloadImageForURLRequest:(NSURLRequest *)request
                                                        success:(nullable void (^)(NSURLRequest *request, NSHTTPURLResponse  * _Nullable response, UIImage *responseObject))success
                                                        failure:(nullable void (^)(NSURLRequest *request, NSHTTPURLResponse * _Nullable response, NSError *error))failure;

/**
 Creates a data task using the `sessionManager` instance for the specified URL request.

 If the same data task is already in the queue or currently being downloaded, the success and failure blocks are
 appended to the already existing task. Once the task completes, all success or failure blocks attached to the
 task are executed in the order they were added.

 @param request The URL request.
 @param receiptID The identifier to use for the download receipt that will be created for this request. This must be a unique identifier that does not represent any other request.
 @param success A block to be executed when the image data task finishes successfully. This block has no return value and takes three arguments: the request sent from the client, the response received from the server, and the image created from the response data of request. If the image was returned from cache, the response parameter will be `nil`.
 @param failure A block object to be executed when the image data task finishes unsuccessfully, or that finishes successfully. This block has no return value and takes three arguments: the request sent from the client, the response received from the server, and the error object describing the network or parsing error that occurred.

 @return The image download receipt for the data task if available. `nil` if the image is stored in the cache.
 cache and the URL request cache policy allows the cache to be used.
 */
//使用指定的会话管理对象为指定url请求创建数据任务
//如果相同的数据任务已经存在或者开始下载，成功或者失败的block会被加到已经存在的任务上
//当任务完成的时候，会触发所有的成功和失败block
- (nullable AFImageDownloadReceipt *)downloadImageForURLRequest:(NSURLRequest *)request
                                                 withReceiptID:(NSUUID *)receiptID
                                                        success:(nullable void (^)(NSURLRequest *request, NSHTTPURLResponse  * _Nullable response, UIImage *responseObject))success
                                                        failure:(nullable void (^)(NSURLRequest *request, NSHTTPURLResponse * _Nullable response, NSError *error))failure;

/**
 Cancels the data task in the receipt by removing the corresponding success and failure blocks and cancelling the data task if necessary.

 If the data task is pending in the queue, it will be cancelled if no other success and failure blocks are registered with the data task. If the data task is currently executing or is already completed, the success and failure blocks are removed and will not be called when the task finishes.

 @param imageDownloadReceipt The image download receipt to cancel.
 */
//取消AFImageDownloadReceipt中的任务，移除相应的成功失败回调，必要时取消数据任务。
- (void)cancelTaskForImageDownloadReceipt:(AFImageDownloadReceipt *)imageDownloadReceipt;

@end

#endif

NS_ASSUME_NONNULL_END
