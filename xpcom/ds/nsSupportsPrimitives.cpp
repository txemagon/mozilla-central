/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsSupportsPrimitives.h"
#include "nsCRT.h"
#include "nsIAllocator.h"
#include "prprf.h"

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsIDImpl, NS_GET_IID(nsISupportsID))

nsSupportsIDImpl::nsSupportsIDImpl()
    : mData(nsnull)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsIDImpl::~nsSupportsIDImpl()
{
    if(mData)
      nsAllocator::Free(mData);
}

NS_IMETHODIMP nsSupportsIDImpl::GetData(nsID **aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    if(mData)
    {
        *aData = (nsID*) nsAllocator::Clone(mData, sizeof(nsID));
        return *aData ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
    }
    else
    {
        *aData = nsnull;
        return NS_OK;
    }
}

NS_IMETHODIMP nsSupportsIDImpl::SetData(nsID *aData)
{
    if(mData)
      nsAllocator::Free(mData);
    if(aData)
        mData = (nsID*) nsAllocator::Clone(aData, sizeof(nsID));
    else
        mData = nsnull;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsIDImpl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *_retval = nsnull;
    if(mData)
    {
        char * str = mData->ToString();
        if(str)
        {
            *_retval = (char*) nsAllocator::Clone(str, 
                                    (nsCRT::strlen(str)+1)*sizeof(char));
            delete [] str;
        }
    }
    else
    {
      static const char nullStr[] = "null";
      *_retval = (char*) nsAllocator::Clone(nullStr, sizeof(nullStr));
    }
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  


/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsStringImpl, NS_GET_IID(nsISupportsString))

nsSupportsStringImpl::nsSupportsStringImpl()
    : mData(nsnull)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsStringImpl::~nsSupportsStringImpl()
{
    if(mData)
      nsAllocator::Free(mData);
}

NS_IMETHODIMP nsSupportsStringImpl::GetData(char **aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    if(mData)
    {
        *aData = (char*) nsAllocator::Clone(mData,
                                (nsCRT::strlen(mData)+1)*sizeof(char));
        return *aData ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
    }
    else
    {
        *aData = nsnull;
        return NS_OK;
    }
}

NS_IMETHODIMP nsSupportsStringImpl::SetData(char *aData)
{
    if(mData)
      nsAllocator::Free(mData);
    if(aData)
        mData = (char*) nsAllocator::Clone(aData,
                                (nsCRT::strlen(aData)+1)*sizeof(char));
    else
        mData = nsnull;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsStringImpl::toString(char **_retval)
{
    return GetData(_retval);
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsWStringImpl, NS_GET_IID(nsISupportsWString))

nsSupportsWStringImpl::nsSupportsWStringImpl()
    : mData(nsnull)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsWStringImpl::~nsSupportsWStringImpl()
{
    if(mData)
      nsAllocator::Free(mData);
}

NS_IMETHODIMP nsSupportsWStringImpl::GetData(PRUnichar **aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    if(mData)
    {
        *aData = (PRUnichar*) nsAllocator::Clone(mData,
                                (nsCRT::strlen(mData)+1)*sizeof(PRUnichar));
        return *aData ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
    }
    else
    {
        *aData = nsnull;
        return NS_OK;
    }
}

NS_IMETHODIMP nsSupportsWStringImpl::SetData(PRUnichar *aData)
{
    if(mData)
      nsAllocator::Free(mData);
    if(aData)
        mData = (PRUnichar*) nsAllocator::Clone(aData,
                                (nsCRT::strlen(aData)+1)*sizeof(PRUnichar));
    else
        mData = nsnull;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsWStringImpl::toString(PRUnichar **_retval)
{
    return GetData(_retval);
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRBoolImpl, NS_GET_IID(nsISupportsPRBool))

nsSupportsPRBoolImpl::nsSupportsPRBoolImpl()
    : mData(PR_FALSE)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRBoolImpl::~nsSupportsPRBoolImpl() {}

NS_IMETHODIMP nsSupportsPRBoolImpl::GetData(PRBool *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRBoolImpl::SetData(PRBool aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRBoolImpl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    const char * str = mData ? "true" : "false";
    *_retval = (char*) nsAllocator::Clone(str,
                                (nsCRT::strlen(str)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRUint8Impl, NS_GET_IID(nsISupportsPRUint8))

nsSupportsPRUint8Impl::nsSupportsPRUint8Impl()
    : mData(0)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRUint8Impl::~nsSupportsPRUint8Impl() {}

NS_IMETHODIMP nsSupportsPRUint8Impl::GetData(PRUint8 *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRUint8Impl::SetData(PRUint8 aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRUint8Impl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 8;
    char buf[size];

    PR_snprintf(buf, size, "%u", (PRUint16) mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRUint16Impl, NS_GET_IID(nsISupportsPRUint16))

nsSupportsPRUint16Impl::nsSupportsPRUint16Impl()
    : mData(0)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRUint16Impl::~nsSupportsPRUint16Impl() {}

NS_IMETHODIMP nsSupportsPRUint16Impl::GetData(PRUint16 *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRUint16Impl::SetData(PRUint16 aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRUint16Impl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 8;
    char buf[size];

    PR_snprintf(buf, size, "%u", (int) mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRUint32Impl, NS_GET_IID(nsISupportsPRUint32))

nsSupportsPRUint32Impl::nsSupportsPRUint32Impl()
    : mData(0)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRUint32Impl::~nsSupportsPRUint32Impl() {}

NS_IMETHODIMP nsSupportsPRUint32Impl::GetData(PRUint32 *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRUint32Impl::SetData(PRUint32 aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRUint32Impl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 16;
    char buf[size];

    PR_snprintf(buf, size, "%lu", mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRUint64Impl, NS_GET_IID(nsISupportsPRUint64))

nsSupportsPRUint64Impl::nsSupportsPRUint64Impl()
    : mData(LL_ZERO)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRUint64Impl::~nsSupportsPRUint64Impl() {}

NS_IMETHODIMP nsSupportsPRUint64Impl::GetData(PRUint64 *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRUint64Impl::SetData(PRUint64 aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRUint64Impl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 32;
    char buf[size];

    PR_snprintf(buf, size, "%llu", mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRTimeImpl, NS_GET_IID(nsISupportsPRTime))

nsSupportsPRTimeImpl::nsSupportsPRTimeImpl()
    : mData(LL_ZERO)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRTimeImpl::~nsSupportsPRTimeImpl() {}

NS_IMETHODIMP nsSupportsPRTimeImpl::GetData(PRTime *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRTimeImpl::SetData(PRTime aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRTimeImpl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 32;
    char buf[size];

    PR_snprintf(buf, size, "%llu", mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsCharImpl, NS_GET_IID(nsISupportsChar))

nsSupportsCharImpl::nsSupportsCharImpl()
    : mData(0)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsCharImpl::~nsSupportsCharImpl() {}

NS_IMETHODIMP nsSupportsCharImpl::GetData(char *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsCharImpl::SetData(char aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsCharImpl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }

    *_retval = (char*) nsAllocator::Alloc(2*sizeof(char));
    if(!*_retval)
        NS_ERROR_OUT_OF_MEMORY;
    (*_retval)[0] = mData;
    (*_retval)[1] = 0;
    return NS_OK;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRInt16Impl, NS_GET_IID(nsISupportsPRInt16))

nsSupportsPRInt16Impl::nsSupportsPRInt16Impl()
    : mData(0)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRInt16Impl::~nsSupportsPRInt16Impl() {}

NS_IMETHODIMP nsSupportsPRInt16Impl::GetData(PRInt16 *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRInt16Impl::SetData(PRInt16 aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRInt16Impl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 8;
    char buf[size];

    PR_snprintf(buf, size, "%d", mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRInt32Impl, NS_GET_IID(nsISupportsPRInt32))

nsSupportsPRInt32Impl::nsSupportsPRInt32Impl()
    : mData(0)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRInt32Impl::~nsSupportsPRInt32Impl() {}

NS_IMETHODIMP nsSupportsPRInt32Impl::GetData(PRInt32 *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRInt32Impl::SetData(PRInt32 aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRInt32Impl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 16;
    char buf[size];

    PR_snprintf(buf, size, "%ld", mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsPRInt64Impl, NS_GET_IID(nsISupportsPRInt64))

nsSupportsPRInt64Impl::nsSupportsPRInt64Impl()
    : mData(LL_ZERO)
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsPRInt64Impl::~nsSupportsPRInt64Impl() {}

NS_IMETHODIMP nsSupportsPRInt64Impl::GetData(PRInt64 *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRInt64Impl::SetData(PRInt64 aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsPRInt64Impl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 32;
    char buf[size];

    PR_snprintf(buf, size, "%lld", mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsFloatImpl, NS_GET_IID(nsISupportsFloat))

nsSupportsFloatImpl::nsSupportsFloatImpl()
    : mData(float(0.0))
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsFloatImpl::~nsSupportsFloatImpl() {}

NS_IMETHODIMP nsSupportsFloatImpl::GetData(float *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsFloatImpl::SetData(float aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsFloatImpl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 32;
    char buf[size];

    PR_snprintf(buf, size, "%f", (double) mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/

NS_IMPL_ISUPPORTS(nsSupportsDoubleImpl, NS_GET_IID(nsISupportsDouble))

nsSupportsDoubleImpl::nsSupportsDoubleImpl()
    : mData(double(0.0))
{
    NS_INIT_ISUPPORTS();
    NS_ADDREF_THIS();
}

nsSupportsDoubleImpl::~nsSupportsDoubleImpl() {}

NS_IMETHODIMP nsSupportsDoubleImpl::GetData(double *aData)
{
    if(!aData)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    *aData = mData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsDoubleImpl::SetData(double aData)
{
    mData = aData;
    return NS_OK;
}

NS_IMETHODIMP nsSupportsDoubleImpl::toString(char **_retval)
{
    if(!_retval)
    {
        NS_ASSERTION(0,"Bad pointer");
        return NS_ERROR_NULL_POINTER;
    }
    static const int size = 32;
    char buf[size];

    PR_snprintf(buf, size, "%f", mData);

    *_retval = (char*) nsAllocator::Clone(buf,
                                (nsCRT::strlen(buf)+1)*sizeof(char));
    return *_retval ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}  

/***************************************************************************/
